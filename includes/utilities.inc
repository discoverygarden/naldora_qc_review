<?php

/**
 * @file
 * Some utility functions... Great fun.
 */

/**
 * Get the role ID for the role with the given name.
 *
 * @param string $role_name
 *   A role for which to get the ID.
 *
 * @return int|FALSE
 *   The role ID as an integer, or FALSE if the role does not exist (or
 *   otherwise failed to load with user_role_load_by_name()).
 */
function naldora_qc_review_get_rid($role_name) {
  $role = user_role_load_by_name($role_name);
  if ($role) {
    return intval($role->rid);
  }
  return FALSE;
}

/**
 * Get users in the role with the given ID.
 *
 * @param int $role_id
 *   The role ID for which to get a list of users.
 *
 * @return array
 *   An array of users, mapping UIDs to names.
 */
function naldora_qc_review_get_user_options($role_id) {
  $s = db_select('users_roles', 'ur')
    ->fields('ur', array('uid'))
    ->condition('rid', $role_id);
  $a = $s->join('users', 'u', 'ur.uid = %alias.uid');
  $s = $s->fields($a, array('name'))
    ->execute();

  $users = array();
  foreach ($s as $u) {
    $users[$u->uid] = $u->name;
  }

  return $users;
}

/**
 * Grant reviewer approval for the given object.
 *
 * @param AbstractObject $object
 *   The object receiving approval.
 *
 * @return bool
 *   TRUE if the object was not already approved and we set the relationships;
 *   otherwise, FALSE.
 */
function naldora_qc_review_reviewer_approve($object) {
  // Mark reviewer's approval.
  if (naldora_qc_review_is_assigned_reviewer($object->id) && !naldora_qc_review_is_reviewer_approved($object)) {
    $pred_info = array(
      ISLANDORA_RELS_EXT_URI => array(
        'naldoraQCReviewer',
        'naldoraQCReviewerApproved',
      ),
    );
    foreach ($pred_info as $uri => $preds) {
      foreach ($preds as $pred) {
        $object->relationships->remove($uri, $pred);
      }
    }

    global $user;
    $object->relationships->add(ISLANDORA_RELS_EXT_URI, 'naldoraQCReviewer', $user->name, RELS_TYPE_PLAIN_LITERAL);
    $object->relationships->add(ISLANDORA_RELS_EXT_URI, 'naldoraQCReviewerApproved', gmdate('Y-m-d\TH:i:s\Z'), RELS_TYPE_DATETIME);
    return TRUE;
  }
  else {
    return FALSE;
  }
}

/**
 * Grant managerial approval for the given object.
 *
 * @param AbstractObject $object
 *   The object receiving approval.
 *
 * @return bool
 *   TRUE if the object was approved by the reviewer an not the manager and we
 *   set the relationships; otherwise, FALSE.
 */
function naldora_qc_review_manager_approve($object) {
  // Mark manager's approval.
  if (naldora_qc_review_is_reviewer_approved($object) && !naldora_qc_review_is_manager_approved($object)) {
    $pred_info = array(
      ISLANDORA_RELS_EXT_URI => array(
        'naldoraQCManager',
        'naldoraQCManagerApproved',
      ),
      'http://purl.org/ontology/bibo/' => array(
        'status',
      ),
    );
    foreach ($pred_info as $uri => $preds) {
      foreach ($preds as $pred) {
        $object->relationships->remove($uri, $pred);
      }
    }

    global $user;
    $object->relationships->add(ISLANDORA_RELS_EXT_URI, 'naldoraQCManager', $user->name, RELS_TYPE_PLAIN_LITERAL);
    $object->relationships->add(ISLANDORA_RELS_EXT_URI, 'naldoraQCManagerApproved', gmdate('Y-m-d\TH:i:s\Z'), RELS_TYPE_DATETIME);
    $object->relationships->add('http://purl.org/ontology/bibo/', 'status', 'waiting', RELS_TYPE_PLAIN_LITERAL);
    return TRUE;
  }
  else {
    return FALSE;
  }
}

/**
 * Check if an object has received reviewer approval.
 *
 * @param AbstractObject $object
 *   The object to test.
 *
 * @return array
 *   If not approved, an empty array; otherwise, an associative array
 *   containing:
 *   - name: The name of the manager who gave approval.
 *   - date: The date on which the manager gave approval.
 */
function naldora_qc_review_is_reviewer_approved($object) {
  $to_return = array();

  $name = $object->relationships->get(ISLANDORA_RELS_EXT_URI, 'naldoraQCReviewer');
  $date = $object->relationships->get(ISLANDORA_RELS_EXT_URI, 'naldoraQCReviewerApproved');
  if ($name && $date) {
    $name = reset($name);
    $date = reset($date);

    $to_return += array(
      'name' => $name['object']['value'],
      'date' => $date['object']['value'],
    );
  }

  return $to_return;
}

/**
 * Check if an object has received managerial approval.
 *
 * @param AbstractObject $object
 *   The object to test.
 *
 * @return array
 *   If not approved, an empty array; otherwise, an associative array
 *   containing:
 *   - name: The name of the manager who gave approval.
 *   - date: The date on which the manager gave approval.
 */
function naldora_qc_review_is_manager_approved($object) {
  $to_return = array();

  $name = $object->relationships->get(ISLANDORA_RELS_EXT_URI, 'naldoraQCManager');
  $date = $object->relationships->get(ISLANDORA_RELS_EXT_URI, 'naldoraQCManagerApproved');
  if ($name && $date) {
    $name = reset($name);
    $date = reset($date);

    $to_return += array(
      'name' => $name['object']['value'],
      'date' => $date['object']['value'],
    );
  }

  return $to_return;
}

/**
 * Get the QC to which this object belongs.
 *
 * @param string $object_id
 *   The PID for which to get the list.
 *
 * @return NaldoraQCReviewList|FALSE
 *   Either an instance of the list, or boolean FALSE if we could not find a
 *   list.
 */
function naldora_qc_review_get_qc_list($object_id) {
  global $user;
  // Is in a list, the list is one of our QC lists, and the user is associated
  // with the list.
  module_load_include('inc', 'islandora_bookmark', 'includes/api');
  $list_ids = islandora_bookmark_get_bookmarks_visible_to_user($object_id, $user->uid);
  $lists = array_map('Bookmark::getList', $list_ids);
  $qc_lists = array_filter($lists, function ($list) {
    return $list instanceof NaldoraQCReviewList;
  });

  return reset($qc_lists);
}

/**
 * Determine if the current user is assigned as a reviewer of an object.
 *
 * @param string $object_id
 *   A string for which we would like to test if we're a reviewer.
 *
 * @return bool
 *   TRUE if the current user is associated with the QC set to which the object
 *   belongs; otherwise, FALSE.
 */
function naldora_qc_review_is_assigned_reviewer($object_id) {
  global $user;

  $qc_list = naldora_qc_review_get_qc_list($object_id);

  return $qc_list && in_array($user->uid, $qc_list->getUsers());
}
