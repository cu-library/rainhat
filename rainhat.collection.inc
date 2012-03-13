<?php

/**
 * @file
 * Collection Management
 */

//List Fedora Collections
function rainhat_collection_list() {

    $header = array(
      array('data' => 'Name', 'field' => 'name', 'sort' => 'asc'),
      array('data' => 'PID', 'field' => 'pid'),
      array('data' => 'Fedora Connection'),
      array('data' => 'Remove'),
    );

    //Theme the html table
    $html = theme('table',
                  array(
                        'header' => $header,
                        'rows' => NULL,
                        'sticky' => TRUE,                                 //Optional to indicate whether the table headers should be sticky
                        'empty' => 'No Collections...',            //Optional empty text for the table if resultset is empty
                  )
            );

    //Append pager
    $html .= theme('pager',
                  array(
                        'tags' => array()
                  )
            );

    return ($html);

}



