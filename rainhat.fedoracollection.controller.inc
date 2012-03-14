<?php

/**
 * @file
 * Controller class definition file for Fedora Collection entity.
 */

/**
 * Controller Class for Fedora Collections
 */
class RainhatFedoraCollectionController extends DrupalDefaultEntityController {
  public function __construct($entityType) {
    parent::__construct('rainhat_fedora_collection');
  }

  public function create() {
    $fedora_collection = new stdClass();
    $fedora_collection->name = '';
    $fedora_collection->pid_prefix_list = '';
    $fedora_collection->parent_fedora_connection_id = 0;
    $fedora_collection->hooked_type = '';
    return $fedora_collection;
  }

  public function save($fedora_collection) {
    if (empty($fedora_collection->id)) {
      drupal_write_record('rainhat_fedora_collection', $fedora_collection);
      module_invoke_all('entity_insert', $fedora_collection, 'rainhat_fedora_collection');
    }
    else {
      drupal_write_record('rainhat_fedora_collection', $fedora_collection, 'id');
      module_invoke_all('entity_update', $fedora_collection, 'rainhat_fedora_collection');
    }

    return $fedora_collection;
  }

  public function delete($ids) {
    if (empty($ids)) {
      return FALSE;
    }

    $fedora_collections = entity_load('rainhat_fedora_collection', $ids);
    db_delete('rainhat_fedora_collection')
      ->condition('id', $ids, 'IN')
      ->execute();

    // Make sure to invoke the delete hook for each topic.
    foreach ($fedora_collections as $fedora_collection) {
      module_invoke_all('entity_delete', $fedora_collection, 'rainhat_fedora_collection');
    }

    // It's necessary to clear the caches when an entity is deleted.
    cache_clear_all();
    $this->resetCache();
    return TRUE;
  }
}
