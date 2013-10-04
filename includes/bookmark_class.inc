<?php

/**
 * @file
 * Bookmark class type.
 */

class NaldoraQCReviewList extends BookmarkDatabase {
  /**
   * Get the URL to this list.
   *
   * @param bool $absolute
   *   Whether or not to create an absolute URL, or just the site-local URL.
   *
   * @return string
   *   An absolute URL to this bookmark list without langague prefixing if
   *   $absolute is TRUE; otherwise, 'islandora-bookmark/listid/<the list id>'.
   */
  public function getListURL($absolute = TRUE) {
    $url = "islandora-bookmark/listid/{$this->bookmarkId}";
    if ($absolute) {
      return url($url, array(
        'language' => NULL,
        'absolute' => TRUE,
      ));
    }
    else {
      return $url;
    }
  }

  /**
   * Inherits...
   *
   * Additionally, only allow an object to be a member of a single QC set, and
   * and set a relationship on the object representing which list it is a
   * member of.
   *
   * DSMs if the given PID is already a member of a QC list.
   */
  public function addPid($pid) {
    // Set the state on the given object.
    $object = islandora_object_load($pid);
    $existing_relationships = $object->relationships->get(ISLANDORA_RELS_EXT_URI, 'naldoraQCSet');
    if (!$existing_relationships) {
      $object->relationships->add(ISLANDORA_RELS_EXT_URI, 'naldoraQCSet', $this->getListURL(), RELS_TYPE_FULL_URI);
      return parent::addPid($pid);
    }
    else {
      $current_list_relationship = reset($existing_relationships);
      $current_list_url = $current_list_relationship['object']['value'];
      $current_list_id = intval(substr($current_list_url, strrpos('/') + 1));
      $current_list = BookmarkDatabase::getList($current_list_id);
      drupal_set_message(filter_xss(t('@pid is already a member of QC list !current_list_link. Not adding to !new_list_link.', array(
        '@pid' => $pid,
        '!current_list_link' => l($current_list->bookmarkName, $current_list_url),
        '!new_list_link' => l($this->bookmarkName, $this->getListURL()),
      ))), 'warning');
      return NULL;
    }
  }

  /**
   * Inherits...
   *
   * Additionally, removes our relationship from the given object.
   */
  public function removePid($pid) {
    $object = islandora_object_load($pid);
    if ($object) {
      $object->relationships->remove(ISLANDORA_RELS_EXT_URI, 'naldoraQCSet', $this->getListURL(), RELS_TYPE_FULL_URI);
    }
    return parent::removePid($pid);
  }

  /**
   * Inherits.
   */
  protected function getTableHeader() {
    return array(
      'title' => t('Title'),
      'reviewer' => t('Reviewer Approved?'),
      'manager' => t('Manager Approved?'),
    );
  }

  /**
   * Inherits.
   */
  protected function getTableRow($pid) {
    $object = islandora_object_load($pid);
    $reviewer_approved = naldora_qc_review_is_reviewer_approved($object);
    $manager_approved = naldora_qc_review_is_manager_approved($object);

    $to_return = array(
      '#attributes' => array(
        'class' => array(),
      ),
      'title' => l(format_string('@label (@pid)', array(
        '@label' => $object->label,
        '@pid' => $object->id,
      )), "islandora/object/{$object->id}"),
      'reviewer' => ($reviewer_approved ?
        t('Yes') :
        t('No')
      ),
      'manager' => ($manager_approved ?
        t('Yes') :
        t('No')
      ),
    );

    if ($reviewer_approved) {
      $to_return['#attributes']['class'][] = 'reviewer-approved';
    }
    if ($manager_approved) {
      $to_return['#attributes']['class'][] = 'manager-approved';
    }

    return $to_return;
  }

  /**
   * Inherits.
   */
  public function formGet($form, &$form_state) {
    form_load_include($form_state, 'inc', 'naldora_qc_review', 'includes/utilities');
    return parent::formGet($form, $form_state);
  }

  /**
   * Inherits.
   */
  protected function formAddActions(&$form_section, &$form_state) {
    global $user;
    if (in_array($user->uid, $this->getUsers())) {
      // Allow reviewers to approve selected/all.
      $form_section['reviewer'] = array(
        '#type' => 'item',
        '#title' => t('Reviewer Approval'),
        'approve_selected' => array(
          '#type' => 'submit',
          '#value' => t('Approve selected'),
          '#action_validate' => array(
            'formHasObjectsSelected',
          ),
          '#action_submit' => array(
            'formReviewerApproveSelected',
          ),
        ),
        'approve_all' => array(
          '#type' => 'submit',
          '#value' => t('Approve all'),
          '#action_submit' => array(
            'formReviewerApproveAll',
          ),
        ),
      );
    }
    if ($this->managementAccess()) {
      // Allow managers to approve selected/all.
      $form_section['manager'] = array(
        '#type' => 'item',
        '#title' => t('Managerial Approval'),
        'approve_selected' => array(
          '#type' => 'submit',
          '#value' => t('Approve selected'),
          '#action_validate' => array(
            'formHasObjectsSelected',
          ),
          '#action_submit' => array(
            'formManagerApproveSelected',
          ),
        ),
        'approve_all' => array(
          '#type' => 'submit',
          '#value' => t('Approve all'),
          '#action_submit' => array(
            'formManagerApproveAll',
          ),
        ),
      );
    }
  }

  /**
   * Inherits.
   */
  protected function formAddSecondaryActions(&$form, &$form_state) {
    // No-op.
  }

  /**
   * Helper method, to actually set the given PIDs approved.
   */
  protected function formReviewerApprove($pids, &$form_state) {
    $this->formApprove($pids, 'naldora_qc_review_reviewer_approve', $form_state);
  }

  /**
   * One of our '#action_submit' callbacks.
   */
  protected function formReviewerApproveSelected(&$form, &$form_state) {
    $pids = $this->formGetSelected($form_state);
    $this->formReviewerApprove($pids, $form_state);
  }

  /**
   * One of our '#action_submit' callbacks.
   */
  protected function formReviewerApproveAll(&$form, &$form_state) {
    $pids = $this->formGetAll($form_state);
    $this->formReviewerApprove($pids, $form_state);
  }

  /**
   * Helper method, to actually set the given PIDs approved.
   */
  protected function formManagerApprove($pids, &$form_state) {
    $this->formApprove($pids, 'naldora_qc_review_manager_approve', $form_state);
  }

  /**
   * One of our '#action_submit' callbacks.
   */
  protected function formManagerApproveSelected(&$form, &$form_state) {
    $pids = $this->formGetSelected($form_state);
    $this->formManagerApprove($pids, $form_state);
  }

  /**
   * One of our '#action_submit' callbacks.
   */
  protected function formManagerApproveAll(&$form, &$form_state) {
    $pids = $this->formGetAll($form_state);
    $this->formManagerApprove($pids, $form_state);
  }

  /**
   * Helper's helper... Actually run the function which set approval.
   *
   * Because we return TRUE/FALSE if we are able to set approval from
   * naldora_qc_review_reviewer_approve() and
   * naldora_qc_review_manager_approve(), we can handle them in much the same
   * way, and report in a consistent manner.
   *
   * @param array $pids
   *   An array of string, representing object PIDs.
   * @param callable $function
   *   A callable we can use in array_filter(), which evalutes to true when the
   *   PID it is provided is successfully processed.
   */
  protected function formApprove($pids, $function, &$form_state) {
    $objects = array_map('islandora_object_load', $pids);
    $approved = array_filter($objects, $function);
    $approved_pids = array_map(function ($object) {
      return $object->id;
    }, $approved);
    $failed = array_diff($pids, $approved_pids);
    drupal_set_message(format_plural(
      count($approved_pids),
      'Approved 1 object.',
      'Approved @count objects.'
    ));
    if (count($failed) > 0) {
      drupal_set_message(format_plural(
        count($failed),
        'One object failed to be approved, likely due to an incorrect initial state (already approved?): %objects',
        '@count objects failed to be approved, likely due to an incorrect initial state (already approved?): %objects',
        array(
          '%objects' => implode(', ', $failed),
        )
      ), 'warning');
    }
  }
}
