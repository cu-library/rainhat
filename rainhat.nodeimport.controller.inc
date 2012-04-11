<?php

/**
 * @file
 * Controller class definition file for Node Import entity.
 */

/**
 * Controller Class for the Node Import entitiy.
 */
class RainhatNodeImportController extends DrupalDefaultEntityController {
  public function __construct($entityType) {
    parent::__construct('rainhat_node_import');
  }

  public function create() {
    $node_import = new stdClass();
    $node_import->name = '';
    $node_import->parent_fedora_connection_id = 0;
    $node_import->hooked_type = '';
    return $node_import;
  }

  public function save($node_import) {
    if (empty($node_import->id)) {

      drupal_write_record('rainhat_node_import', $node_import);
      module_invoke_all('entity_insert', $node_import, 'rainhat_node_import');

    }
    else {
      drupal_write_record('rainhat_node_import', $node_import, 'id');
      module_invoke_all('entity_update', $node_import, 'rainhat_node_import');
    }

    return $node_import;
  }

  public function delete($ids) {
    if (empty($ids)) {
      return FALSE;
    }

    $node_imports = entity_load('rainhat_node_import', $ids);
    db_delete('rainhat_node_import')
      ->condition('id', $ids, 'IN')
      ->execute();

    foreach ($node_imports as $node_import) {
      module_invoke_all('entity_delete', $node_import, 'rainhat_node_import');
    }

    //Because field imports live as children of the node import, we need
    // to delete them so they aren't 'orphaned' in the database.
    // Though this sounds cruel, it's really for the best.
    $affected_field_imports = db_select('rainhat_field_import', 'rfi')
      ->fields('rfi', array('id'))
      ->condition('parent_node_import_id', $ids, 'IN')
      ->execute();

    foreach ($affected_field_imports as $affected_field_import) {
      entity_get_controller('rainhat_field_import')->delete(array($affected_field_import->id));
    }

    // It's necessary to clear the caches when an entity is deleted.
    cache_clear_all();
    $this->resetCache();
    return TRUE;
  }
}
