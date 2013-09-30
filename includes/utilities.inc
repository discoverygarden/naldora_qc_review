<?php

/**
 * @file
 * Some utility functions... Great fun.
 */

define('NALDORA_QC_REVIEW_QC_WAITING', 'QC-waiting');
define('NALDORA_QC_REVIEW_QC_REVIEW', 'QC-review');
define('NALDORA_QC_REVIEW_QC_FINAL', 'QC-final-ck');
define('NALDORA_QC_REVIEW_AI_WAITING', 'waiting');

/**
 * Set the Quality Control review state of the given object.
 *
 * @param AbstractObject $object
 *   The object on which to set the state.
 * @param string $state
 *   The state to set.
 */
function naldora_qc_review_set_state(AbstractObject $object, $state) {
  // TODO: Set the state relationship.

  if ($state == NALDORA_QC_REVIEW_QC_FINAL) {
    // TODO: Set the relationship to indicate when something passed review.
  }
  elseif ($state == NALDORA_QC_REVIEW_AI_WAITING) {
    // TODO, maybe?: Set relationship to track final approval date.
  }
}

/**
 * Set the Quality Control reviewer for the given object.
 *
 * @param NaldoraQCReviewList $bookmark_list
 *   The list of bookmarks to assign.
 * @param object $reviewer
 *   A Drupal user-object to whom to assign the object.
 */
function naldora_qc_review_set_reviewer(NaldoraQCReviewList $bookmark_list, $reviewer) {

}

/**
 * Wrapper for relationship adding/setting...
 *
 * Ensure only a single copy of the given relationship exists.
 *
 * @param AbstractObject $object
 *   The object on which to make the relationship.
 * @param string $uri
 *   The URI to which the relationship belongs.
 * @param string $pred
 *   The predicate of the relationship.
 * @param mixed $obj
 *   The object of the relationship.
 * @param int $type
 *   The type of $obj, as defined by Tuque.
 */
function naldora_qc_review_set_rels_ext(AbstractObject $object, $uri, $pred, $obj, $type) {

}
