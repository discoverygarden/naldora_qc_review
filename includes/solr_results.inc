<?php

/**
 * @file
 * Contains methods to search solr and display results. Depends on
 * Apache_Solr_Php client.
 */

/**
 * Extention of IslandoraSolrResults to create an alternative display type.
 */
class NaldoraQCSolrReviewResults extends IslandoraSolrResultsBookmark {
  /**
   * Get the header row to use in the tableselect.
   *
   * @return array
   *   An associative array as used as headers by theme_tableselect()/
   */
  protected function getTableHeader() {
    return array(
      'title' => t('Title'),
      'host_title' => t('Host Title'),
      'volume_issue' => t('Volume/Issue'),
      'date_issued' => t('Date Issued'),
    );
  }

  /**
   * Build a row for inclusion in the tableselect.
   *
   * @param array $object_result
   *   A result from the query processor.
   *
   * @return array
   *   An associative array of cells, keyed according to the header provided in
   *   $this->getTableHeader().
   */
  protected function getTableRow($object_result) {
    $doc = $object_result['solr_doc'];

    $map = array(
      'title' => array(
        'field' => 'mods_titleInfo_title_ms',
        'link' => TRUE,
      ),
      'host_title' => array(
        'field' => 'mods_relatedItem_host_titleInfo_title_ms',
        'link' => FALSE,
      ),
      'volume_issue' => array(
        'field' => 'mods_relatedItem_volume_issue_ms',
        'link' => FALSE,
      ),
      'date_issued' => array(
        'field' => 'mods_originInfo_dateIssued_mdt',
        'link' => FALSE,
      ),
    );

    $map_function = function ($field_info) use ($doc) {
      extract($field_info);

      $to_implode = isset($doc[$field]) ?
        (array) $doc[$field] :
        array();
      $text = implode(', ', $to_implode);

      return $link ?
        l($text, "islandora/object/{$doc['PID']}") :
        $text;
    };

    return array_map($map_function, $map);
  }

  /**
   * Get the "actions" section of the form.
   *
   * @return array
   *   A form fragment containing a number of actions which might be performed
   *   on objects from the table select.
   *
   *   Submit buttons may have two additional properties:
   *   - action_validate: An array of validation methods to call, which should
   *     form_set_error() as necessary.
   *   - action_submit: An array of submission methods to call.
   *   as hinted, these are /methods/, which should exist on the given class
   *   (or one of its ancestors).
   */
  protected function getActions(&$form_state) {
    global $user;

    $groups = array();

    if (in_array(NALDORA_QC_REVIEW_MANAGER_ROLE, $user->roles)) {
      // Get list of users with our role.
      module_load_include('inc', 'naldora_qc_review', 'includes/utilities');
      $reviewers = naldora_qc_review_get_user_options(naldora_qc_review_get_rid(NALDORA_QC_REVIEW_REVIEWER_ROLE));

      $groups['naldora_qc_review'] = array(
        '#type' => 'fieldset',
        '#title' => t('Quality Control Assignment'),
        'reviewer' => array(
          '#type' => 'select',
          '#title' => t('Reviewer'),
          '#options' => array('default' => t('-- Select Reviewer --')) + $reviewers,
        ),
        'assign_selected' => array(
          '#type' => 'submit',
          '#value' => t('Assign Selected'),
          '#action_validate' => array(
            'hasObjectsSelected',
            'hasReviewerSelected',
          ),
          '#action_submit' => array(
            'saveSelected',
          ),
        ),
        'assign_all' => array(
          '#type' => 'submit',
          '#value' => t('Assign All'),
          '#action_validate' => array(
            'hasReviewerSelected',
          ),
          '#action_submit' => array(
            'saveAll',
          ),
        ),
      );
    }

    return $groups;
  }

  /**
   * One of our '#action_validate' callbacks.
   *
   * Checks that a reviewer has been selected.
   */
  protected function hasReviewerSelected($form, &$form_state) {
    $el = $form_state['complete form']['naldora_qc_review']['reviewer'];
    $key = drupal_array_get_nested_value($form_state['values'], $el['#parents']);

    if ($key == 'default') {
      form_set_error(implode('][', $el['#parents']), t('You must select a reviewer to whom to assign the set.'));
    }
  }

  /**
   * One of our '#action_submit' callbacks.
   *
   * Save all PIDs the search yielded to the selected list.
   *
   * @param array $form_state
   *   The array of form state.
   */
  protected function saveAll(&$form_state) {
    $pids = $this->getAll($form_state);
    $this->save($pids, $form_state);
  }

  /**
   * One of our '#action_submit' callbacks.
   *
   * Save the selected PIDs the search yielded to the selected list.
   *
   * @param array $form_state
   *   The array of form state.
   */
  protected function saveSelected(&$form_state) {
    $pids = $this->getSelected($form_state);
    $this->save($pids, $form_state);
  }

  /**
   * Actually handle saving PIDs to a list.
   *
   * @param array $pids_to_add
   *   An array of PIDs to save to the given list.
   * @param array $form_state
   *   The array of form state.
   */
  protected function save($pids_to_add, &$form_state) {
    $reviewer_el = $form_state['complete form']['naldora_qc_review']['reviewer'];
    $reviewer = drupal_array_get_nested_value($form_state['values'], $reviewer_el['#parents']);
    $reviewer_object = user_load($reviewer);

    $list_name = format_string('QC set created @date, and assigned to @name.', array(
      '@date' => gmdate('Y-m-d\TH:i:s\Z'),
      '@name' => $reviewer_object->name,
    ));

    if (count($pids_to_add) > 0) {
      // Create $bookmark_object, to which to add our PIDs.
      $bookmark_object = BookmarkDatabase::createNewList($list_name, NALDORA_QC_REVIEW_LIST_TYPE);

      // Add PIDs to the list.
      try {
        $bookmark_object->addPids($pids_to_add);

        drupal_set_message(format_plural(
          count($pids_to_add),
          'The selected object has been added to "@listname".',
          'The selected objects have been added to "@listname".',
          array(
            '@listname' => $bookmark_object->bookmarkName,
          )
        ));
      }
      catch (Exception $e) {
        drupal_set_message($e->getMessage(), 'error');
        return;
      }

      // Associate the reviewer with the bookmark object.
      $bookmark_object->addUser($reviewer_object->uid);
      $form_state['redirect'] = $bookmark_object->getListURL(FALSE);
    }
  }
}
