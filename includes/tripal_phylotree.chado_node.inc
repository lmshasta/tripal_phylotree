<?php

/**
 * @file
 * Implements the phylotree node content type
 */

/**
 * Implements hook_node_info().
 *
 * Provide information to drupal about the node types that we're creating
 * in this module.
 *
 * @ingroup tripal_phylotree
 */
function tripal_phylotree_node_info() {
  $nodes = array();
  $nodes['chado_phylotree'] = array(
    'name'        => t('Phylotree'),
    'base'        => 'chado_phylotree',
    'description' => t('A phylotree from the chado database'),
    'has_title'   => TRUE,
    'locked'      => TRUE,
    'chado_node_api' => array(
      'base_table' => 'phylotree',
      'hook_prefix' => 'chado_phylotree',
      'record_type_title' => array(
        'singular' => t('Phylotree'),
        'plural' => t('Phylotrees')
      ),

      /* sync_filters: tripal is hardcoded to look for this
       sync_filter settings: type_id and organism_id. (phylotree does
       not have organism_id but need to set it false anyways. */
      'sync_filters' => array(
        'type_id' => FALSE,
        'organism_id' => FALSE
      ),
    )
  );
  return $nodes;
}


/**
 * Implements hook_node_view(). Acts on all content types
 *
 * @ingroup tripal_phylotree
 */
function tripal_phylotree_node_view($node, $view_mode, $langcode) {

  if($node->type != 'chado_phylotree') { return; }

  switch($view_mode) {
  case 'full':
    $node->content['tripal_phylotree_base'] = array(
      '#theme' => 'tripal_phylotree_base',
      '#node' => $node,
      '#tripal_toc_id'    => 'base',
      '#tripal_toc_title' => 'Overview',
      '#weight' => -100,
    );
    $node->content['tripal_phylotree_phylogram'] = array(
      '#theme' => 'tripal_phylotree_phylogram',
      '#node' => $node,
      '#tripal_toc_id'    => 'phylotree_phylogram',
      '#tripal_toc_title' => 'Phylogram',
      '#weight' => -90,
    );
    $node->content['tripal_phylotree_taxonomic_tree'] = array(
      '#theme' => 'tripal_phylotree_taxonomic_tree',
      '#node' => $node,
      '#tripal_toc_id'    => 'tripal_phylotree_taxonomic_tree',
      '#tripal_toc_title' => 'Taxonomic Tree',
      '#weight' => -80,
    );
    $node->content['tripal_phylotree_radial'] = array(
      '#theme' => 'tripal_phylotree_radial',
      '#node' => $node,
      '#tripal_toc_id'    => 'phylotree_circ_dendrogram',
      '#tripal_toc_title' => 'Circular Dendrogram',
      '#weight' => -80,
    );
    $node->content['tripal_phylotree_organisms'] = array(
      '#theme' => 'tripal_phylotree_organisms',
      '#node' => $node,
      '#tripal_toc_id'    => 'phylotree_organisms',
      '#tripal_toc_title' => 'Organisms',
      '#weight' => -70,
    );
    $node->content['tripal_phylotree_references'] = array(
      '#theme' => 'tripal_phylotree_references',
      '#node' => $node,
      '#tripal_toc_id'    => 'phylotree_references',
      '#tripal_toc_title' => 'Cross References',
    );
     $node->content['tripal_phylotree_analysis'] = array(
      '#theme' => 'tripal_phylotree_analysis',
      '#node' => $node,
      '#tripal_toc_id'    => 'phylotree_analysis',
      '#tripal_toc_title' => 'Analysis',
    );
    break;

  case 'teaser':
    $node->content['tripal_phylotree_teaser'] = array(
      '#theme' => 'tripal_phylotree_teaser',
      '#node' => $node,
    );
    break;
    }
}

/**
 * Implementation of hook_form().
 *
 * @ingroup tripal_phylotree
 */
function chado_phylotree_form($node, &$form_state) {

  $form = array();

  // Default values can come in the following ways:
  //
  // 1) as elements of the $node object.  This occurs when editing an existing phylotree
  // 2) in the $form_state['values'] array which occurs on a failed validation or
  //    ajax callbacks from non submit form elements
  // 3) in the $form_state['input'[ array which occurs on ajax callbacks from submit
  //    form elements and the form is being rebuilt
  //
  // set form field defaults
  $phylotree     = null;
  $phylotree_id  = null;
  $tree_name     = '';
  $leaf_type     = '';
  $analysis_id   = '';
  $dbxref        = '';
  $comment       = '';
  $tree_required = TRUE;
  $tree_file     = '';
  $name_re      = '';
  $match = '';

  // If we are editing an existing node then the phylotree is already part of the node.
  if (property_exists($node, 'phylotree')) {
    $phylotree      = $node->phylotree;
    $phylotree      = chado_expand_var($phylotree, 'field', 'phylotree.comment');
    $phylotree_id   = $phylotree->phylotree_id;
    $tree_name      = $phylotree->name;
    $leaf_type      = $phylotree->type_id ? $phylotree->type_id->name : '';
    $comment        = $phylotree->comment;
    $analysis_id    = $phylotree->analysis_id ? $phylotree->analysis_id->analysis_id : '';
    $dbxref         = $phylotree->dbxref_id->db_id->name . ":" . $phylotree->dbxref_id->accession;
    $name_re        = $phylotree->tripal_variables->phylotree_name_re;
    $match          = $phylotree->tripal_variables->phylotree_use_uniquename;

    // If the dbxref is the null db then hide it.
    if ($phylotree->dbxref_id->db_id->name == 'null') {
      $dbxref = '';
    }

    // Get the tree file name. If the file was added via the Drupal interface
    // then a numeric file_id will be present in the phylotree_tree_file
    // variable. If not then the tree was loaded on the command-line and
    // the actual filename is in this variable.
    $file_id = $phylotree->tripal_variables->phylotree_tree_file;
    if (is_numeric($file_id)) {
      $file = file_load($file_id);
      if ($file) {
        $tree_file = $file->filename;
      }
    }
    else {
      $tree_file = $file_id;
    }

    // The tree file is not a required input field when editing the node.
    $tree_required  = FALSE;

    // Keep track of the phylotree id.
    $form['phylotree_id'] = array(
      '#type' => 'value',
      '#value' => $phylotree_id,
    );
  }
  // If we are re constructing the form from a failed validation or ajax callback
  // then use the $form_state['values'] values.
  if (array_key_exists('values', $form_state) and isset($form_state['values']['tree_name'])) {
    $tree_name    = $form_state['values']['tree_name'];
    $leaf_type    = $form_state['values']['leaf_type'];
    $analysis_id  = $form_state['values']['analysis_id'];
    $dbxref       = $form_state['values']['dbxref'];
    $comment      = $form_state['values']['description'];
  }
  // If we are re building the form from after submission (from ajax call) then
  // the values are in the $form_state['input'] array.
  if (array_key_exists('input', $form_state) and !empty($form_state['input'])) {
    $tree_name    = $form_state['input']['tree_name'];
    $leaf_type    = $form_state['input']['leaf_type'];
    $analysis_id  = $form_state['input']['analysis_id'];
    $comment      = $form_state['input']['description'];
    $dbxref       = $form_state['input']['dbxref'];
  }

  $form['tree_name']= array(
    '#type' => 'textfield',
    '#title' => t('Tree Name'),
    '#required' => TRUE,
    '#default_value' => $tree_name,
    '#description' => t('Enter the name used to refer to this phylogenetic tree.'),
    '#maxlength' => 255
  );

  $type_cv = tripal_get_default_cv('phylotree', 'type_id');
  $so_cv  = tripal_get_cv(array('name' => 'sequence'));
  $cv_id = $so_cv->cv_id;
  if (!$so_cv) {
    drupal_set_message('The Sequence Ontolgoy does not appear to be imported.
        Please import the Sequence Ontology before adding a tree.', 'error');
  }

  $form['leaf_type'] = array(
    '#title'       => t('Tree Type'),
    '#type'        => 'textfield',
    '#description' => t("Choose the tree type. The type is
        a valid Sequence Ontology (SO) term. For example, trees derived
        from protein sequences should use the SO term 'polypeptide'.
        Alternatively, a phylotree can be used for representing a taxonomic
        tree. In this case, the word 'taxonomy' should be used."),
    '#required'    => TRUE,
    '#default_value' => $leaf_type,
    '#autocomplete_path' => "admin/tripal/chado/tripal_cv/cvterm/auto_name/$cv_id",
  );

  // Get the list of analyses.
  $sql = "SELECT * FROM {analysis} ORDER BY name";
  $arset = chado_query($sql);
  $analyses = array();
  $analyses[''] = '';
  while ($analysis = $arset->fetchObject()) {
    $analyses[$analysis->analysis_id] = $analysis->name;
  }
  $form['analysis_id'] = array(
    '#title'         => t('Analysis'),
    '#type'          => 'select',
    '#description'   => t("Choose the analysis from which this phylogenetic tree was derived"),
    '#required'      => TRUE,
    '#default_value' => $analysis_id,
    '#options'       => $analyses,
  );

  $form['dbxref'] = array(
    '#title'       => t('Database Cross-Reference'),
    '#type'        => 'textfield',
    '#description' => t("Enter a database cross-reference of the form
        [DB name]:[accession]. The database name must already exist in the
        database. If the accession does not exist it is automatically added."),
    '#required'    => FALSE,
    '#default_value' => $dbxref,
  );

  $form['description']= array(
    '#type' => 'textarea',
    '#title' => t('Description'),
    '#required' => TRUE,
    '#default_value' => $comment,
    '#description' => t('Enter a description for this tree.'),
  );

  $upload_location = tripal_get_files_stream('tripal_phylotree');
  $form['tree_file'] = array(
    '#type' => 'fieldset',
    '#title' => t('Tree File Import'),
    '#collapsible' => FALSE,
  );

  $description = t('Please provide a file in the Newick format that contains
      the nodes of this tree.');
  if ($tree_file) {
    $form['tree_file']['curr_file'] = array(
      '#type' => 'item',
      '#title' => 'Current Tree File',
      '#markup' => $tree_file,
    );
    $description = t('Please provide a file in the Newick format that
        contains the nodes of this tree.  Please note that uploading a new
        file will overwrite the current tree.');
  }
  $form['tree_file']['tree_file'] = array(
    '#type' => 'managed_file',
    '#title' => t('New Tree File'),
    '#description' => $description,
    '#upload_location' => $upload_location,
    '#upload_validators' => array(
        // We don't want to require a specific file extension so leave the array empty.
       'file_validate_extensions' => array(),
        // The following is for checking the Newick file format.
       'chado_phylotree_validate_newick_format' => array(),
    ),
    '#required' => $tree_required,
  );

  $form['tree_file']['name_re'] = array(
      '#title' => t('Feature Name Regular Expression'),
      '#type' => 'textfield',
      '#description' => t('If this is a phylogenetic (non taxonomic) tree, then
          the tree nodes will be automatically associated with features. However,
          if the nodes in the tree file are not exactly as the names of features
          but have enough information to uniquely identify the feature then you
          may provide a regular expression that the importer will use to extract
          the feature names from the node names.'),
      '#default_value' => $name_re,
  );
  $form['tree_file']['match'] = array(
      '#title' => t('Use Unique Feature Name'),
      '#type' => 'checkbox',
      '#description' => t('If this is a phylogenetic (non taonomic tree) and the nodes ' .
          'should match the unique name of the feature rather than the name of the feautre ' .
          'then select this box. If unselected the loader will try to match the feature ' .
          'using the feature name.'),
      '#default_value' => $match,
  );

  return $form;
}

/**
 * A validation function for checking the newick file format.
 *
 * @param stdClass $file
 *   A Drupal file object.
 */
function chado_phylotree_validate_newick_format(stdClass $file) {
  // An array of strings where each string represents a unique error
  // when examining the file.
  $errors = array();

  // TODO: check the newick file format for errors.

  return $errors;
}

/**
 * Implementation of hook_validate().
 *
 * This validation is being used for three activities:
 *   CASE A: Update a node that exists in both drupal and chado
 *   CASE B: Synchronizing a node from chado to drupal
 *   CASE C: Inserting a new node that exists in niether drupal nor chado
 *
 * @ingroup tripal_phylotree
 */
function chado_phylotree_validate($node, $form, &$form_state) {

  // We are syncing if we do not have a node ID but we do have a phylotree_id. We don't
  // need to validate during syncing so just skip it.
  if (is_null($node->nid) and property_exists($node, 'phylotree_id') and $node->phylotree_id != 0) {
    return;
  }

  // Remove surrounding white-space on submitted values.
  $node->tree_name  = trim($node->tree_name);
  $node->description   = trim($node->description);
  $node->dbxref = trim($node->dbxref);

  // if this is a delete then don't validate
  if ($node->op == 'Delete') {
    return;
  }

  $errors = array();
  $options = array(
    'name'         => $node->tree_name,
    'description'  => $node->description,
    'analysis_id'  => $node->analysis_id,
    'leaf_type'    => $node->leaf_type,
    'tree_file'    => $node->tree_file,
    'format'       => 'newick',
    'dbxref'       => $node->dbxref,
    'match'        => $node->match,
    'name_re'      => $node->name_re,
  );
  // If we have a node id already then this is an update:
  if ($node->nid) {
    $options['phylotree_id'] = $node->phylotree_id;
    tripal_validate_phylotree('update', $options, $errors);
  }
  else {
    tripal_validate_phylotree('insert', $options, $errors);
  }

  // Now set form errors if any errors were detected.
  if (count($errors) > 0) {
    foreach($errors as $field => $message) {
      if ($field == 'name') {
        $field = 'tree_name';
      }
      form_set_error($field, $message);
    }
  }
}
/**
 * Implements hook_node_presave(). Acts on all node content types.
 *
 * @ingroup tripal_phylotree
 */
function tripal_phylotree_node_presave($node) {

  switch ($node->type) {
    // This step is for setting the title for the Drupal node.  This title
    // is permanent and thus is created to be unique.  Title changes provided
    // by tokens are generated on the fly dynamically, but the node title
    // seen in the content listing needs to be set here. Do not call
    // the chado_get_node_title() function here to set the title as the node
    // object isn't properly filled out and the function will fail.
    case 'chado_phylotree':
      // for a form submission the 'phylotreename' field will be set,
      // for a sync, we must pull from the phylotree object
      if (property_exists($node, 'phylotreename')) {
        // set the title
        $node->title = $node->tree_name;
      }
      else if (property_exists($node, 'phylotree')) {
        $node->title = $node->phylotree->name;
      }
      break;
  }
}

/**
 * Implements hook_node_insert().
 * Acts on all content types.
 *
 * @ingroup tripal_phylotree
 */
function tripal_phylotree_node_insert($node) {

  switch ($node->type) {
    case 'chado_phylotree':

      $phylotree_id = chado_get_id_from_nid('phylotree', $node->nid);
      $values = array('phylotree_id' => $phylotree_id);
      $phylotree = chado_generate_var('phylotree', $values);
      $phylotree = chado_expand_var($phylotree, 'field', 'phylotree.comment');
      $node->phylotree = $phylotree;

      // Now use the API to set the path.
      chado_set_node_url($node);

      // Now get the title.
      $node->title = chado_get_node_title($node);

      break;
  }
}

/**
 * Implements hook_node_update().
 * Acts on all content types.
 *
 * @ingroup tripal_phylotree
 */
function tripal_phylotree_node_update($node) {

  switch ($node->type) {
    case 'chado_phylotree':

      $phylotree_id = chado_get_id_from_nid('phylotree', $node->nid);
      $values = array('phylotree_id' => $phylotree_id);
      $phylotree = chado_generate_var('phylotree', $values);
      $phylotree = chado_expand_var($phylotree, 'field', 'phylotree.comment');
      $node->phylotree = $phylotree;

      // Now get the title
      $node->title = chado_get_node_title($node);

      break;
  }
}

/**
 * Implements [content_type]_chado_node_default_title_format().
 *
 * Defines a default title format for the Chado Node API to set the titles on
 * Chado phylotree nodes based on chado fields.
 *
 * @ingroup tripal_phylotree
 */
function chado_phylotree_chado_node_default_title_format() {
  return '[phylotree.name]';
}

/**
 * Implements hook_chado_node_default_url_format().
 *
 * Designates a default URL format for phylotree nodes.
 */
function chado_phylotree_chado_node_default_url_format() {
  return '/phylotree/[phylotree.name]';
}

/**
 *  Implements hook_insert().
 *
 *  When a new chado_phylotree node is created we also need to add
 *  information to our chado_phylotree table.  This function is called
 *  on insert of a new node of type 'chado_phylotree' and inserts the
 *  necessary information.
 *
 * @ingroup tripal_phylotree
 */
function chado_phylotree_insert($node) {
  global $user;

  $node->tree_name   = trim($node->tree_name);
  $node->description = trim($node->description);
  $node->dbxref      = trim($node->dbxref);

  // if there is a phylotree_id in the $node object then this must
  // be a sync (not an insert) so we can skip adding the phylotree as it is
  // already there, although we do need to proceed with the rest of the
  // insert.
  $phylotree_id = NULL;
  if (!property_exists($node, 'phylotree_id')) {
    $options = array(
      'name'         => $node->tree_name,
      'description'  => $node->description,
      'analysis_id'  => $node->analysis_id,
      'leaf_type'    => $node->leaf_type,
      'tree_file'    => $node->tree_file,
      'format'       => 'newick',
      'dbxref'       => $node->dbxref,
      'match'        => $node->match,
      'name_re'      => $node->name_re,
    );
    if (tripal_insert_phylotree($options)) {
      $phylotree_id = $options['phylotree_id'];

      // Add the Tripal variables to this node.
      tripal_add_node_variable($node->nid, 'phylotree_name_re', $node->name_re);
      tripal_add_node_variable($node->nid, 'phylotree_use_uniquename', $node->match);
      tripal_add_node_variable($node->nid, 'phylotree_tree_file', $node->tree_file);
    }
    else {
      drupal_set_message(t('Unable to insert phylotree.'), 'error');
      tripal_report_error('tripal_phylotree', TRIPAL_WARNING,
          'Insert phylotree: Unable to inesrt phylotree where values: %values',
          array('%values' => print_r($options, TRUE))
      );
    }
  }
  else {
    $phylotree_id = $node->phylotree_id;
  }

  // Make sure the entry for this phylotree doesn't already exist in the
  // chado_phylotree table if it doesn't exist then we want to add it.
  $check_org_id = chado_get_id_from_nid('phylotree', $node->nid);
  if (!$check_org_id) {
    $record = new stdClass();
    $record->nid = $node->nid;
    $record->vid = $node->vid;
    $record->phylotree_id = $phylotree_id;
    drupal_write_record('chado_phylotree', $record);
  }
}

/**
 * Implements hook_update().
 *
 * @ingroup tripal_phylotree
 */
function chado_phylotree_update($node) {

  global $user;

  $node->tree_name  = trim($node->tree_name);
  $node->description   = trim($node->description);
  $node->dbxref = trim($node->dbxref);

  // Get the phylotree_id for this node.
  $phylotree_id = chado_get_id_from_nid('phylotree', $node->nid) ;


  $options = array(
    'phylotree_id' => $node->phylotree_id,
    'name'         => $node->tree_name,
    'description'  => $node->description,
    'analysis_id'  => $node->analysis_id,
    'leaf_type'    => $node->leaf_type,
    'tree_file'    => $node->tree_file,
    'format'       => 'newick',
    'dbxref'       => $node->dbxref,
    'match'        => $node->match,
    'name_re'      => $node->name_re,
  );

  $success = tripal_update_phylotree($phylotree_id, $options);

  if (!$success) {
    drupal_set_message("Unable to update phylotree.", "error");
    tripal_report_error('tripal_phylotree', TRIPAL_WARNING,
      'Update phylotree: Unable to update phylotree where values: %values',
      array('%values' => print_r($options, TRUE))
    );
    return;
  }

  // Remove any variables and then add back the variables from the form.
  tripal_delete_node_variables($node->nid);
  tripal_add_node_variable($node->nid, 'phylotree_name_re', $node->name_re);
  tripal_add_node_variable($node->nid, 'phylotree_use_uniquename', $node->match);
  tripal_add_node_variable($node->nid, 'phylotree_tree_file', $node->tree_file);
}
/**
 *  Implements hook_load().
 *
 *  When a node is requested by the user this function is called to allow us
 *  to add auxiliary data to the node object.
 *
 * @ingroup tripal_phylotree
 */
function chado_phylotree_load($nodes) {

  foreach ($nodes as $nid => $node) {

    $phylotree_id = chado_get_id_from_nid('phylotree', $nid);

    // If the nid does not have a matching record then skip this node.
    // this can happen with orphaned nodes.
    if (!$phylotree_id) {
      continue;
    }

    // Build the Chado variable for the phylotree.
    $values = array('phylotree_id' => $phylotree_id);
    $phylotree = chado_generate_var('phylotree', $values);
    $nodes[$nid]->phylotree = $phylotree;

    // Expand the comment field, chado_generate_var() omits it by default
    // because it is a large text field.
    $phylotree = chado_expand_var($phylotree, 'field', 'phylotree.comment');

    // Add non Chado information to the object. These variables are needed
    // for the edit/update forms.
    $phylotree->tripal_variables = new stdClass;
    $variables = tripal_get_node_variables($nid, 'phylotree_name_re');
    $phylotree->tripal_variables->phylotree_name_re = count($variables) > 0 ? $variables[0]->value : '';

    $variables = tripal_get_node_variables($nid, 'phylotree_use_uniquename');
    $phylotree->tripal_variables->phylotree_use_uniquename = count($variables) > 0 ? $variables[0]->value : '';

    $variables = tripal_get_node_variables($nid, 'phylotree_tree_file');
    $phylotree->tripal_variables->phylotree_tree_file = count($variables) > 0 ? $variables[0]->value : '';

    // Set the title for this node.
    $node->title = chado_get_node_title($node);
  }
}

/**
 * Implements hook_delete().
 *
 * Delete data from drupal and chado databases when a node is deleted
 *
 * @ingroup tripal_phylotree
 */
function chado_phylotree_delete(&$node) {

  $phylotree_id = chado_get_id_from_nid('phylotree', $node->nid);

  // if we don't have a phylotree id for this node then this isn't a node of
  // type chado_phylotree or the entry in the chado_phylotree table was lost.
  if (!$phylotree_id) {
    return;
  }

  // Remove data from {chado_phylotree}, {node} and {node_revisions} tables of
  // drupal database
  $sql_del = "DELETE FROM {chado_phylotree} WHERE nid = :nid AND vid = :vid";
  db_query($sql_del, array(':nid' => $node->nid, ':vid' => $node->vid));
  $sql_del = "DELETE FROM {node_revision} WHERE nid = :nid AND vid = :vid";
  db_query($sql_del, array(':nid' => $node->nid, ':vid' => $node->vid));
  $sql_del = "DELETE FROM {node} WHERE nid = :nid AND vid = :vid";
  db_query($sql_del, array(':nid' => $node->nid, ':vid' => $node->vid));

  // Remove data from phylotree and phylotreeprop tables of chado
  // database as well

  chado_query("DELETE FROM {phylotree} WHERE phylotree_id = :phylotree_id", array(':phylotree_id' => $phylotree_id));

}

/**
 * Implement hook_node_access().
 *
 * This hook allows node modules to limit access to the node types they define.
 *
 *  @param $node
 *  The node on which the operation is to be performed, or, if it does not yet exist, the
 *  type of node to be created
 *
 *  @param $op
 *  The operation to be performed
 *
 *  @param $account
 *  A user object representing the user for whom the operation is to be performed
 *
 *  @return
 *  If the permission for the specified operation is not set then return FALSE. If the
 *  permission is set then return NULL as this allows other modules to disable
 *  access.  The only exception is when the $op == 'create'.  We will always
 *  return TRUE if the permission is set.
 *
 * @ingroup tripal_phylotree
 */
function chado_phylotree_node_access($node, $op, $account) {

  $node_type = $node;
  if (is_object($node)) {
    $node_type = $node->type;
  }

  if($node_type == 'chado_phylotree') {
    if ($op == 'create') {
      if (!user_access('create chado_phylotree content', $account)) {
        return NODE_ACCESS_DENY;
      }
      return NODE_ACCESS_ALLOW;
    }
    if ($op == 'update') {
      if (!user_access('edit chado_phylotree content', $account)) {
        return NODE_ACCESS_DENY;
      }
    }
    if ($op == 'delete') {
      if (!user_access('delete chado_phylotree content', $account)) {
        return NODE_ACCESS_DENY;
      }
    }
    if ($op == 'view') {
      if (!user_access('access chado_phylotree content', $account)) {
        return NODE_ACCESS_DENY;
      }
    }
    return NODE_ACCESS_IGNORE;
  }
}

/**
 * phylotree_by_name
 *
 * Lookup tree by name, and redirect to that drupal node. This could
 * also be done for example by URL aliases, for example using pathauto
 * module.
 *
 * @param int $phylotree_id
 * @return string json
 * @ingroup tripal_phylotree
 */
function phylotree_by_name($name) {

    $sql = <<<SQL
 SELECT cp.nid as node_id
 FROM chado.phylotree t
 LEFT JOIN chado_phylotree cp on cp.phylotree_id = t.phylotree_id
 WHERE t.name = :name
SQL;

    $args = array(':name' => $name);
    $result = chado_query( $sql, $args );
    $nid = $result->fetchField();
    drupal_goto("node/$nid");
}

/**
 * phylotree_json
 *  Get json representation of a phylotree id. See menu item for JSON service
 *  endpoint.
 *
 * @param int $phylotree_id
 * @return string json
 * @ingroup tripal_phylotree
 */
function phylotree_json($phylotree_id) {

  $phylotree = chado_generate_var('phylotree', array('phylotree_id' => $phylotree_id));

  // This SQL gets all of the phylonodes for a given tree as well as the
  // features and organisms with which it is assocaited.  Each phylonode
  // can be associated with an orgnaism in one of two ways: 1) via a
  // feature linked by the phylonode.feature_id field or 2) via a
  // a record in the phylonde_organsim table.  Therefore both types of
  // organism records are returned in the query below, but those
  // retrieved via a FK link on features are prefixed with 'fo_'.
  $sql = "
    SELECT
      n.phylonode_id, n.parent_phylonode_id, n.label AS name, n.distance AS length,
      f.feature_id, f.name AS feature_name,
      cvt.name AS cvterm_name,
      o.organism_id, o.common_name, o.abbreviation, o.genus, o.species,
      fo.organism_id AS fo_organism_id, fo.common_name AS fo_common_name,
      fo.abbreviation AS fo_abbreviation, fo.genus as fo_genus, fo.species AS fo_species,
      cf.nid AS feature_node_id,
      fco.nid AS fo_organism_node_id,
      co.nid AS organism_node_id
    FROM {phylonode} n
      LEFT OUTER JOIN {cvterm} cvt              ON n.type_id = cvt.cvterm_id
      LEFT OUTER JOIN {feature} f               ON n.feature_id = f.feature_id
      LEFT OUTER JOIN public.chado_feature cf   ON cf.feature_id = f.feature_id
      LEFT OUTER JOIN {organism} fo             ON f.organism_id = fo.organism_id
      LEFT OUTER JOIN public.chado_organism fco ON fco.organism_id = fo.organism_id
      LEFT OUTER JOIN {phylonode_organism} po   ON po.phylonode_id = n.phylonode_id
      LEFT OUTER JOIN {organism} o              ON PO.organism_id = o.organism_id
      LEFT OUTER JOIN public.chado_organism co  ON co.organism_id = o.organism_id
    WHERE n.phylotree_id = :phylotree_id
  ";
  $args = array(':phylotree_id' => $phylotree_id);
  $result = chado_query($sql, $args);

  // Fetch all the phylonodes into an assoc array indexed by phylonode_id.
  // Convert from resultset record to array, fixing datatypes. chado_query
  // returns numeric as string and fun stuff like that.
  $phylonodes = array();
  $root_phylonode_ref = null;

  // Get the tree properties
  $root_size = variable_get('tripal_phylotree_default_root_node_size', 5);
  $internal_size = variable_get('tripal_phylotree_default_internal_node_size', 5);
  $leaf_size = variable_get('tripal_phylotree_default_leaf_node_size', 5);


  foreach ($result as $r) {
    $phylonode_id = (int) $r->phylonode_id;

    // expect all nodes to have these properties
    $node = array(
      'phylonode_id' => $phylonode_id,
      'parent_phylonode_id' => (int) $r->parent_phylonode_id,
      'length' => (double) $r->length,
      'cvterm_name' => $r->cvterm_name
    );

    // If the nodes are taxonomic then set an equal distnace
    if ($phylotree->type_id->name == 'taxonomy') {
        $node['length'] = 0.001;
    }

    // Other props may exist only for leaf nodes
    if ($r->name) {
      $node['name'] = $r->name;
    }
    // If this node is associated with a feature then add in the details
    if ($r->feature_id) {
      $node['feature_id'] = (int) $r->feature_id;
      $node['feature_name'] = $r->feature_name;
      $node['feature_node_id'] = (int) $r->feature_node_id;
    }
    // Add in the organism fields when they are available via the
    // phylonode_organism table.
    if ($r->organism_id) {
      $node['organism_id'] = (int) $r->organism_id;
      $node['common_name'] = $r->common_name;
      $node['abbreviation'] = $r->abbreviation;
      $node['genus'] = $r->genus;
      $node['species'] = $r->species;
      $node['organism_node_id'] = (int) $r->organism_node_id;
      // If the node does not have a name but is linked to an organism
      // then set the name to be that of the genus and species.
      if (!$r->name) {
        $node['name'] = $r->genus . ' ' . $r->species;
      }
    }
    // Add in the organism fields when they are available via the
    // the phylonode.feature_id FK relationship.
    if ($r->fo_organism_id) {
      $node['fo_organism_id'] = (int) $r->fo_organism_id;
      $node['fo_common_name'] = $r->fo_common_name;
      $node['fo_abbreviation'] = $r->fo_abbreviation;
      $node['fo_genus'] = $r->fo_genus;
      $node['fo_species'] = $r->fo_species;
      $node['fo_organism_node_id'] = (int) $r->fo_organism_node_id;
    }

    // Add this node to the list, organized by ID.
    $phylonodes[$phylonode_id] = $node;
  }
  drupal_debug($phylonodes);

  // Populate the children[] arrays for each node.
  foreach ($phylonodes as $key => &$node) {
    if ($node['parent_phylonode_id'] !== 0) {
      $parent_ref = &$phylonodes[ $node['parent_phylonode_id']];
      // Append node refernce to children.
      $parent_ref['children'][] = &$node;
    }
    else {
      $root_phylonode_ref = &$node;
    }
  }

  // dump datastructure as json to browser. drupal sets the mime-type correctly.
  drupal_json_output($root_phylonode_ref);
}

/**
 *  Phylotree feature summary.
 *
 *  Get an array of feature counts by organism. key = organism
 *  abbreviation. value = number of features for this phylotree having
 *  this organism.
 *
 * @param int phylotree_id
 * @return array
 * @ingroup tripal_phylotree
 */
function phylotree_feature_summary($phylotree_id) {

  $sql = "
    SELECT o.abbreviation, COUNT(o.organism_id) AS count
    FROM {phylonode} n
      LEFT OUTER JOIN {feature} f  ON n.feature_id = f.feature_id
      LEFT OUTER JOIN {organism} o ON f.organism_id = o.organism_id
    WHERE n.phylotree_id = :phylotree_id
      AND n.feature_id IS NOT NULL
    GROUP BY o.organism_id
  ";

  $args = array(':phylotree_id' => $phylotree_id);
  $result = chado_query($sql, $args);
  $summary = array();
  foreach($result as $r) {
    $summary[$r->abbreviation] = $r->count;
  }
  return $summary;
}
