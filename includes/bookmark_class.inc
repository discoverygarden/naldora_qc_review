<?php

class NaldoraQCReviewList extends BookmarkDatabase {
  protected function getListURL() {
    return url("islandora-bookmark/listid/{$this->bookmarkId}", array(
      'absolute' => TRUE,
    ));
  }

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

  public function removePid($pid) {
    $object = islandora_object_load($pid);
    $object->relationships->remove(ISLANDORA_RELS_EXT_URI, 'naldoraQCSet', $this->getListURL(), RELS_TYPE_FULL_URI);
    return parent::removePid($pid);
  }

  public function addUser($user_id) {
    // Set user as the reviewer.
    // TODO: Notify user?
    return parent::addUser($user_id);
  }

  public function removeUser($user_id) {
    // TODO: Remove user as reviewer... Review complete on this list?
    // TODO: Iterate over every PID, setting state to completed?...
    return parent::removeUser($user_id);
  }
}
