<?php

class NaldoraQCReviewList extends BookmarkDatabase {
  protected function addPid($pid) {
    // TODO: Set the state on the given object.
    return parent::addPid($pid);
  }

  public function removePid($pid) {
    trigger_error('Unsupported action; cannot remove PIDs from NaldoraQCReviewLists.');
  }

  public function addUser($user_id) {
    // TODO: Set user as the reviewer.
    return parent::addUser($user_id);
  }

  public function removeUser($user_id) {
    // TODO: Remove user as reviewer... Review complete on this list?
    // TODO: Iterate over every PID, setting state to completed?...
    return parent::removeUser($user_id);
  }
}
