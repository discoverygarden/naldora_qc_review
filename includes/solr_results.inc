<?php

/**
 * @file
 * Contains methods to search solr and display results. Depends on
 * Apache_Solr_Php client.
 */

/**
 * Extention of IslandoraSolrResults to create an alternative display type.
 */
class NaldoraQCSolrResults extends IslandoraSolrResultsBookmark {
  /**
   * Get the header row to use in the tableselect.
   *
   * @return array
   *   An associative array as used as headers by theme_tableselect()/
   */
  protected function getTableHeader() {
    return array(
      'markup' => t('Label'),
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

    $markup = '';
    if (isset($doc['object_label'])) {
      $markup = $doc['object_label'];
    }
    else {
      $markup = $doc['PID'];
    }

    $object_url_info = array(
      'path' => $object_result['object_url'],
      'params' => $object_result['object_url_params'],
      'markup' => $markup,
    );

    return array(
      'markup' => islandora_bookmark_generate_markup($doc['PID'], $object_url_info),
    );
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
    $groups = array();

    // TODO: Get list of users with whatever role...
    $reviewers = array();
    $groups['naldora_qc_review'] = array(
      '#type' => 'fieldset',
      '#title' => t('Quality Control Assignment'),
      'reviewer' => array(
        '#type' => 'select',
        '#title' => t('Reviewer'),
        '#options' => array('default' => t('-- Select Reviewer --') + $reviewers,
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

    return $groups;
  }

  /**
   * One of our '#action_validate' callbacks.
   *
   * Checks that a reviewer has been selected.
   */
  protected function hasReviewerSelected($form, $form_state) {
    $el = $form_state['complete form']['naldora_qc_review']['reviewer'];
    $key = drupal_array_get_nested_value($form_state['values'], $el['#parents']);

    if ($key == 'default') {
      form_set_error(implode('][', $el['#parents']), t('You must select a reviewer to whom to assign the set.'));
    }
  }

  /**
   * One of our '#action_validate' callbacks.
   *
   * Checks that some objects are selected.
   */
  protected function hasObjectsSelected($form, $form_state) {
    $pids = $this->getSelected($form_state);
    if (empty($pids)) {
      form_set_error('islandora_bookmark_table', t('You must select at least one object.'));
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
  protected function saveAll($form_state) {
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
  protected function saveSelected($form_state) {
    $pids = $this->getSelected($form_state);
    $this->save($pids, $form_state);
  }

  /**
   * Actually handle saving PIDs to a list.
   *
   * @param array $pids
   *   An array of PIDs to save to the given list.
   * @param array $form_state
   *   The array of form state.
   */
  protected function save($pids_to_add, $form_state) {
    $reviewer_el = $form_state['complete form']['naldora_qc_review']['reviewer'];
    $reviewer = drupal_array_get_nested_value($form_state['values'], $reviewer_el['#parents']);
    $reviwer_object = user_load($reviewer);

    $list_name_el = $form_state['complete form']['naldora_qc_review']['list_name'];
    $list_name = drupal_array_get_nested_value($form_state['values'], $list_name_el['#parents']);

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

      // Assign the $bookmark_object to the selected reviewer.
      module_load_include('inc', 'naldora_qc_review', 'includes/utilities');
      naldora_qc_review_set_reviewer($bookmark_object, $reviewer_object);
    }
  }
}