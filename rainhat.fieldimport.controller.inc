<?php

/**
 * @file
 * Controller class definition file for the Field Import entity.
 */

/**
 * Controller Class for Field Imports
 */
class RainhatFieldImportController extends DrupalDefaultEntityController {
  public function __construct($entityType) {
    parent::__construct('rainhat_field_import');
  }

  public function create($node_import_id, $field) {
    $field_import = new stdClass();
    $field_import->field_id = $field;
    $field_import->type = '';
    $field_import->php_script = '';
    $field_import->datastream_id = '';
    $field_import->parent_node_import_id = $node_import_id;

    return $field_import;
  }

  public function save($field_import) {

    if (empty($field_import->id)) {
      drupal_write_record('rainhat_field_import', $field_import);
      module_invoke_all('entity_insert', $field_import, 'rainhat_field_import');
    }
    else {
      drupal_write_record('rainhat_field_import', $field_import, 'id');
      module_invoke_all('entity_update', $field_import, 'rainhat_field_import');
    }

    return $field_import;
  }

  public function delete($ids) {
    if (empty($ids)) {
      return FALSE;
    }

    $field_imports = entity_load('rainhat_field_import', $ids);
    db_delete('rainhat_field_import')
      ->condition('id', $ids, 'IN')
      ->execute();

    // Make sure to invoke the delete hook for each topic.
    foreach ($field_imports as $field_import) {
      module_invoke_all('entity_delete', $field_import, 'rainhat_field_import');
    }

    // It's necessary to clear the caches when an entity is deleted.
    cache_clear_all();
    $this->resetCache();
    return TRUE;
  }
}
