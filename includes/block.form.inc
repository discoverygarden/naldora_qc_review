<?php

/**
 * @file
 * Form to go in the block on object pages, for QC reviewers and managers.
 */

/**
 * Form building function.
 *
 * @param string $object_id
 *   The object for which to render the form.
 */
function naldora_qc_review_form($form, &$form_state, $object_id) {
  global $user;
  form_load_include($form_state, 'inc', 'naldora_qc_review', 'includes/block.form');
  form_load_include($form_state, 'inc', 'naldora_qc_review', 'includes/utilities');

  $possible_list = naldora_qc_review_get_qc_list($object_id);
  $object = islandora_object_load($object_id);
  if ($possible_list) {
    $approval = naldora_qc_review_is_reviewer_approved($object);
    $manager_approval = naldora_qc_review_is_manager_approved($object);

    if (in_array($user->uid, $possible_list->getUsers())) {
      // User is the/a reviewer.
      if ($approval) {
        // You approved this on such and such...
        $form['reviewer_approval'] = array(
          '#type' => 'item',
          '#title' => t('Reviewer approval'),
          '#markup' => t('You approved this item on @date.', array(
            '@name' => $approval['name'],
            '@date' => $approval['date'],
          )),
        );
        $form['manager_approval'] = array(
          '#type' => 'item',
          '#title' => t('Manager approval'),
          '#markup' => ($manager_approval ?
            t('This item received managerial approval from @name on @date.', array(
              '@name' => $manager_approval['name'],
              '@date' => $manager_approval['date'],
            )) :
            t('This item has not yet been approved by a manager.')
          ),
        );
      }
      else {
        // Allow assigned reviewer to approve.
        $form['reviewer_approval'] = array(
          '#type' => 'submit',
          '#value' => t('Reviewer: Approve'),
        );
      }
    }
    elseif ($possible_list->managementAccess()) {
      // User is the manager/list owner.
      if ($approval) {
        // Allow manager to provide final approval, if approved by reviewer.
        $form['reviewer_approval'] = array(
          '#type' => 'item',
          '#title' => t('Reviewer approval'),
          '#markup' => t('Approved by @name on @date.', array(
            '@name' => $approval['name'],
            '@date' => $approval['date'],
          )),
        );
        if ($manager_approval) {
          $form['manager_approval'] = array(
            '#type' => 'item',
            '#title' => t('Manager approval'),
            '#markup' => t('You provided managerial approval for this item on @date.', array(
              '@date' => $manager_approval['date'],
            )),
          );
        }
        else {
          $form['manager_approval'] = array(
            '#type' => 'submit',
            '#value' => t('Manager: Approve'),
          );
        }
      }
      else {
        $users = array_map('user_load', $possible_list->getUsers());
        $users = array_filter($users);
        $names = array_map(function ($user) {
          return $user->name;
        }, $users);
        $form['reviewer_approval'] = array(
          '#type' => 'item',
          '#title' => t('Reviewer approval'),
          '#markup' => t('This item has not yet been approved. It is in %list, which is assigned to: @name.', array(
            '%list' => $possible_list->bookmarkName,
            '@name' => implode(', ', $names),
          )),
        );
        $form['manager_approval'] = array(
          '#type' => 'item',
          '#title' => t('Manager approval'),
          '#markup' => t('This item is not yet eligible for approval, as it has not been reviewed by a reviewer.'),
        );
      }
    }
  }

  return $form;
}

/**
 * Form submission handler.
 */
function naldora_qc_review_form_submit(&$form, &$form_state) {
  global $user;
  $button = end($form_state['triggering_element']['#parents']);
  $object = islandora_object_load($form_state['build_info']['args'][0]);
  if ($button == 'reviewer_approval') {
    naldora_qc_review_reviewer_approve($object);
  }
  elseif ($button == 'manager_approval') {
    naldora_qc_review_manager_approve($object);
  }
}
