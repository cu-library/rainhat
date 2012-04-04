<?php

/**
 * @file
 * Controller class definition file for Fedora Connection entity.
 */

/**
 * Controller Class for Fedora Connections
 */
class RainhatFedoraConnectionController extends DrupalDefaultEntityController {
  public function __construct($entityType) {
    parent::__construct('rainhat_fedora_connection');
  }

  public function create() {
    $fedora_connection = new stdClass();
    $fedora_connection->name = '';
    $fedora_connection->url = variable_get('rainhat_default_fedora_url');
    $fedora_connection->admin_username = variable_get('rainhat_default_fedora_admin_username');
    $fedora_connection->admin_password = variable_get('rainhat_default_fedora_admin_password');
    return $fedora_connection;
  }

  public function save($fedora_connection) {
    if (empty($fedora_connection->id)) {
      drupal_write_record('rainhat_fedora_connection', $fedora_connection);
      module_invoke_all('entity_insert', $fedora_connection, 'rainhat_fedora_connection');
    }
    else {
      drupal_write_record('rainhat_fedora_connection', $fedora_connection, 'id');
      module_invoke_all('entity_update', $fedora_connection, 'rainhat_fedora_connection');
    }

    return $fedora_connection;
  }

  public function delete($ids) {
    if (empty($ids)) {
      return FALSE;
    }

    $fedora_connections = entity_load('rainhat_fedora_connection', $ids);
    db_delete('rainhat_fedora_connection')
      ->condition('id', $ids, 'IN')
      ->execute();

    // Make sure to invoke the delete hook for each topic.
    foreach ($fedora_connections as $fedora_connection) {
      module_invoke_all('entity_delete', $fedora_connection, 'rainhat_fedora_connection');
    }

    //Because node exports live as children of the fedora connection, we need
    // to delete them so they aren't 'orphaned' in the database.
    // Though this sounds cruel, it's really for the best.
    $affected_node_exports = db_select('rainhat_node_export', 'rne')
      ->fields('rne', array('id'))
      ->condition('parent_fedora_connection_id', $ids, 'IN')
      ->execute();

    foreach ($affected_node_exports as $affected_node_export) {
      entity_get_controller('rainhat_fedora_connection')->delete(array($affected_node_export->id));
    }

    // It's necessary to clear the caches when an entity is deleted.
    cache_clear_all();
    $this->resetCache();
    return TRUE;
  }
}
