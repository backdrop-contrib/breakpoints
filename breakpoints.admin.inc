<?php

/**
 * @file
 * Breakpoints - admin settings
 */

/**
 * Admin form
 */
function breakpoints_admin_breakpoints($form, &$form_state, $breakpoint_group_name = '') {

  // Global is the same as no group name
  $global = FALSE;
  if ($breakpoint_group_name == '' || $breakpoint_group_name == 'global') {
    $breakpoint_group_name = '';
    $global = TRUE;
  }

  $form_state['group_name'] = $breakpoint_group_name;

  $form = array();

  $settings = breakpoints_settings();
  $multipliers = array();
  if (isset($settings->multipliers) && !empty($settings->multipliers)) {
    $multipliers = drupal_map_assoc(array_values($settings->multipliers));
    if (array_key_exists('1x', $multipliers)) {
      unset($multipliers['1x']);
    }
  }

  if ($global) {
    $form['info'] = array(
      '#type' => 'markup',
      '#markup' => t("You can manage all your breakpoints on this screen, if one of your themes has breakpoints defined inside the .info file they will be shown here."),
    );

    $info = array();
    $info[] = t("To create a new breakpoint, you have to enter a name and a media query (ex. (min-width: 15em)).");
    $info[] = t("All breakpoints can be enabled or disabled so they cannot be used by other modules.");
    $info[] = t("For each breakpoint you can define what multipliers have to be available (needed to support 'retina' displays).");
    $info[] = t("Breakpoints you created yourself can be deleted.");
    $info[] = t("You can group multiple breakpoints in a group by using '!add', so other modules can easily interact with them.", array('!add' => l(t('Add a new group'), 'admin/config/media/breakpoints/groups/add')));

    $form['more_info'] = array(
      '#type' => 'container',
      '#theme' => 'item_list',
      '#items' => $info,
    );
  }
  else {
    $form['info'] = array(
      '#type' => 'markup',
      '#markup' => t("You can manage the breakpoints of this group here."),
    );
    $info = array();
    $info[] = t("You can change the order of the breakpoints inside this group.");
    $info[] = t("You can enable multipliers for each breakpoint, but this will also affect other groups.");

    $form['more_info'] = array(
      '#type' => 'container',
      '#theme' => 'item_list',
      '#items' => $info,
    );
  }

  $form['breakpoints'] = array(
    '#type' => 'container',
    '#tree' => TRUE,
    '#theme' => 'breakpoints_admin_breakpoints_table',
    '#multipliers' => $multipliers,
    '#group_name' => $breakpoint_group_name,
  );

  $breakpoints = array();
  $breakpoint_group = breakpoints_breakpoint_group_load($breakpoint_group_name);
  if ($global) {
    $breakpoints = breakpoints_breakpoint_load_all();
  }
  else {
    $weight = 0;
    foreach ($breakpoint_group->breakpoints as $breakpoint_name) {
      $breakpoint = breakpoints_breakpoint_load_by_fullkey($breakpoint_name);
      if ($breakpoint && isset($breakpoint->machine_name)) {
        $breakpoint->global_weight = $breakpoint->weight;
        $breakpoint->weight = $weight++;
        $breakpoints[$breakpoint_name] = $breakpoint;
      }
    }
  }

  foreach ($breakpoints as $key => $breakpoint) {
    $form['breakpoints'][$key] = array(
      '#breakpoint_data' => $breakpoint,
      'name' => array(
        '#type' => 'textfield',
        '#default_value' => $breakpoint->name,
        '#disabled' => TRUE,
        '#size' => 20,
      ),
      'breakpoint' => array(
        '#type' => 'textfield',
        '#default_value' => $breakpoint->breakpoint,
        '#disabled' => $breakpoint->source_type === 'theme' || !$global,
        '#size' => empty($multipliers) ? 60 : 30,
      ),
      'weight' => array(
        '#type' => 'textfield',
        '#size' => 4,
        '#default_value' => isset($breakpoint->weight) ? $breakpoint->weight : 0,
        '#attributes' => array('class' => array('breakpoints-weight')),
      ),
    );
    // Add multipliers checkboxes if needed.
    if (!empty($multipliers)) {
      $form['breakpoints'][$key]['multipliers'] = array(
        '#type' => 'checkboxes',
        '#default_value' => (isset($breakpoint->multipliers) && is_array($breakpoint->multipliers)) ? $breakpoint->multipliers : array(),
        '#options' => $multipliers,
      );
    }
  }

  if ($global) {
    // Add empty row
    $form['breakpoints']['new'] = array(
      'name' => array(
        '#type' => 'textfield',
        '#default_value' => '',
        '#size' => 20,
      ),
      'machine_name' => array(
        '#type' => 'machine_name',
        '#size' => '64',
        '#title' => t('Machine name'),
        '#default_value' => '',
        '#machine_name' => array(
          'exists' => 'breakpoints_breakpoint_name_exists',
          'source' => array('breakpoints', 'new', 'name'),
        ),
      ),
      'breakpoint' => array(
        '#type' => 'textfield',
        '#default_value' => '',
        '#size' => empty($multipliers) ? 60 : 30,
      ),
      'weight' => array(
        '#type' => 'textfield',
        '#size' => 4,
        '#default_value' => 0,
        '#attributes' => array('class' => array('breakpoints-weight')),
      ),
    );
    // Add multipliers checkboxes if needed.
    if (!empty($multipliers)) {
      $form['breakpoints']['new']['multipliers'] = array(
        '#type' => 'checkboxes',
        '#default_value' => array(),
        '#options' => $multipliers,
      );
    }
  }

  // Buttons
  $form['buttons'] = array(
    '#type' => 'container',
  );

  // Submit button
  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  if (!$global) {
    $form['buttons']['editlink'] = array(
      '#type' => 'markup',
      '#markup' => l(t('Edit group breakpoints'), 'admin/config/media/breakpoints/groups/' . $breakpoint_group_name . '/edit', array(
        'query' => drupal_get_destination(),
      )),
    );
    if ($breakpoint_group->type == BREAKPOINTS_SOURCE_TYPE_CUSTOM) {
      $form['buttons']['deletelink'] = array(
        '#type' => 'markup',
        '#markup' => l(t('Delete this group'), 'admin/config/media/breakpoints/groups/' . $breakpoint_group_name . '/delete', array(
          'query' => array('destination' => 'admin/config/media/breakpoints/groups'),
        )),
      );
    }
  }

  return $form;
}

/**
 * Theme form as table.
 */
function theme_breakpoints_admin_breakpoints_table($variables) {
  drupal_add_css(drupal_get_path('module', 'breakpoints') . '/css/breakpoints.admin.css');
  $form = $variables['form'];
  $rows = array();
  $link_attributes = array(
    'attributes' => array(
      'class' => array('image-style-link'),
    ),
  );
  foreach (element_children($form) as $key) {
    $form[$key]['weight']['#attributes']['class'] = array('breakpoints-weight');
    $row = array();
    if ($key != 'new') {
      $row[] = drupal_render($form[$key]['name']);
    }
    else {
      $row[] = drupal_render($form[$key]['name']) . ' ' . drupal_render($form[$key]['machine_name']);
    }
    $row[] = drupal_render($form[$key]['breakpoint']);
    $row[] = drupal_render($form[$key]['multipliers']);
    if ($key != 'new') {
      $breakpoint = $form[$key]['#breakpoint_data'];
      $row[] = $breakpoint->source . ' (' . $breakpoint->source_type . ')';
      $row[] = $breakpoint->status ? t('Enabled') : t('Disabled');
      if (empty($form['#group_name'])) {
        $row[] = l($breakpoint->status ? t('Disable') : t('Enable'), 'admin/config/media/breakpoints/' . ($breakpoint->status ? 'disable' : 'enable') . '/' . $key, $link_attributes);
      }
      if (empty($form['#group_name'])) {
        if ($breakpoint->source == 'user') {
          $row[] = l(t('Delete'), 'admin/config/media/breakpoints/delete/' . $key, $link_attributes);
        }
        else {
          $row[] = '';
        }
        $row[] = l(t('Export'), 'admin/config/media/breakpoints/export/' . $key, $link_attributes);
      }
      $class = 'resp-img-status-' . ($breakpoint->status ? 'enabled' : 'disabled');
    }
    else {
      $row[] = '';
      $row[] = '';
      $row[] = '';
      $row[] = '';
      $row[] = '';
      $class = 'resp-img-status-new';
    }
    $row[] = drupal_render($form[$key]['weight']);
    $rows[] = array(
      'data' => $row,
      'class' => array('draggable', $class),
    );
  }

  if (empty($form['#group_name'])) {
    $header = array(
      t('Name'),
      t('Breakpoint, @media ...'),
      t('Multipliers'),
      t('Source'),
      t('Status'),
      array('data' => t('Operations'), 'colspan' => 3),
      t('Weight'),
    );
  }
  else {
    $header = array(
      t('Name'),
      t('Breakpoint, @media ...'),
      t('Multipliers'),
      t('Source'),
      t('Status'),
      t('Weight'),
    );
  }

  $output = theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'resp-img-breakpoints')));
  drupal_add_tabledrag('resp-img-breakpoints', 'order', 'sibling', 'breakpoints-weight');

  return $output;
}

/**
 * Admin form validation.
 */
function breakpoints_admin_breakpoints_validate($form, &$form_state) {
  if (strpos($form_state['triggering_element']['#id'], 'remove') === FALSE) {
    $breakpoints = $form_state['values']['breakpoints'];
    if (!empty($breakpoints)) {
      foreach ($breakpoints as $key => $breakpointdata) {
        if (!empty($breakpointdata['name'])) {
          // Breakpoint is required.
          if ($key == 'new') {
            if (empty($breakpointdata['machine_name'])) {
              form_set_error('breakpoints][' . $key . '][machine_name', 'Required');
            }
          }
          if (empty($breakpointdata['breakpoint']) && $breakpointdata['breakpoint'] !== '0') {
            form_set_error('breakpoints][' . $key . '][breakpoint', 'Required');
          }
        }
      }
    }
  }
}

/**
 * Admin form submit.
 */
function breakpoints_admin_breakpoints_submit($form, &$form_state) {
  $breakpoints = $form_state['values']['breakpoints'];
  $group_name = $form_state['group_name'];
  $global_group = $group_name == '';
  if (!$global_group) {
    // sort by weight, needed to store the right order in a group
    uasort($breakpoints, '_breakpoints_sort_by_weight_array');
  }
  $saved_breakpoints = array();
  if (!empty($breakpoints)) {
    foreach ($breakpoints as $breakpointname => $breakpointdata) {
      if (!empty($breakpointdata['name'])) {
        $breakpoint = breakpoints_breakpoint_load_by_fullkey($breakpointname);
        if ($breakpoint && $breakpointname != 'new') {
          // only save the weight when on the global screen.
          if ($global_group) {
            $breakpoint->weight = $breakpointdata['weight'];
          }
          $breakpoint->breakpoint = $breakpointdata['breakpoint'];
          $breakpoint->multipliers = $breakpointdata['multipliers'];
          breakpoints_breakpoint_save($breakpoint);
          $saved_breakpoints[] = $breakpointname;
        }
        else {
          $breakpoint = new stdClass();
          $breakpoint->name = $breakpointdata['name'];
          $breakpoint->breakpoint = $breakpointdata['breakpoint'];
          $breakpoint->source = 'user';
          $breakpoint->source_type = 'custom';
          $breakpoint->weight = $breakpointdata['weight'];
          $breakpoint->status = TRUE;
          $breakpoint->multipliers = $breakpointdata['multipliers'];
          $breakpoint->machine_name = 'custom.user.' . $breakpointdata['machine_name'];
          breakpoints_breakpoint_save($breakpoint);
          $saved_breakpoints[] = breakpoints_breakpoint_config_name($breakpoint);
        }
      }
    }
    if (!$global_group) {
      $group = breakpoints_breakpoint_group_load($group_name);
      if ($group) {
        $group->breakpoints = $saved_breakpoints;
        breakpoints_breakpoint_group_save($group);
      }
    }
  }
}

/**
 * Admin action form: enable, disable, delete
 */
function breakpoints_admin_breakpoint_actions_form($form, &$form_state, $group_name, $action, $breakpoint) {
  switch ($action) {
    case 'enable':
    case 'disable':
    case 'delete':
      $form_state['group_name'] = $group_name;
      $form_state['action'] = $action;
      $form_state['breakpoint'] = $breakpoint;
      $question = t('Are you sure you want to %action %breakpoint', array(
            '%action' => $action,
            '%breakpoint' => $breakpoint,
          ));
      if (!empty($group_name)) {
        $path = 'admin/config/media/breakpoints/groups/' . $group_name;
      }
      else {
        $path = 'admin/config/media/breakpoints';
      }
      $form = confirm_form($form, $question, $path, '');
      break;
    case 'export':
      $form = drupal_get_form('breakpoints_admin_breakpoint_export_form', $breakpoint);
      break;
  }
  return $form;
}

/**
 * Admin action form submit
 */
function breakpoints_admin_breakpoint_actions_form_submit($form, &$form_state) {
  $group_name = $form_state['group_name'];
  $action = $form_state['action'];
  $breakpoint = $form_state['breakpoint'];
  switch ($action) {
    case 'delete':
      breakpoints_breakpoint_delete_by_fullkey($breakpoint);
      break;
    case 'enable':
    case 'disable':
      breakpoints_breakpoint_toggle_status($breakpoint);
      break;
  }
  if (!empty($group_name)) {
    $form_state['redirect'] = 'admin/config/media/breakpoints/groups/' . $group_name;
  }
  else {
    $form_state['redirect'] = 'admin/config/media/breakpoints';
  }
}

function breakpoints_add_style_form($form, &$form_state) {
  module_load_include('inc', 'image', 'image.admin');
  $form['style'] = array(
    '#title' => t('Image style'),
    '#type' => 'select',
    '#options' => image_style_options(FALSE),
    '#required' => TRUE,
    '#description' => t('This image style will be cloned to create the responsive style'),
  );

  $form['base_name'] = array(
    '#type' => 'textfield',
    '#size' => '64',
    '#title' => t('Image style base name'),
    '#description' => t('The name is used in URLs for generated images. Use only lowercase alphanumeric characters, underscores (_), and hyphens (-).'),
    '#element_validate' => array('image_style_name_validate'),
    '#required' => TRUE,
  );
  $breakpoints = breakpoints_breakpoint_load_all_active();
  $options = array();
  foreach ($breakpoints as $breakpoint) {
    $options[$breakpoint->breakpoint] = $breakpoint->name . ' (' . $breakpoint->breakpoint . ')';
  }
  $form['breakpoints'] = array(
    '#title' => t('breakpoints'),
    '#type' => 'checkboxes',
    '#options' => $options,
    '#default_value' => drupal_map_assoc(array_keys($options)),
    '#description' => t('Select the breakpoints to create an image style for'),
    '#required' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Create'),
  );
  return $form;
}

function breakpoints_add_style_form_validate($form, &$form_state) {
  foreach (array_filter($form_state['values']['breakpoints']) as $breakpoint) {
    if (image_style_load($form_state['values']['base_name'] . $breakpoint)) {
      form_set_error('breakpoints', t('An image style with the name !name already exists', array('!name' => $form_state['values']['base_name'] . $breakpoint)));
    }
  }
}

function breakpoints_add_style_form_submit($form, &$form_state) {
  $base = image_style_load($form_state['values']['style']);
  if (!isset($base['effects'])) {
    $base['effects'] = array();
  }
  foreach (array_filter($form_state['values']['breakpoints']) as $breakpoint) {
    $new_style = array(
      'name' => $form_state['values']['base_name'] . $breakpoint,
      'effects' => $base['effects'],
    );
    image_style_save($new_style);
  }
  $form_state['redirect'] = 'admin/config/media/image-styles';
  drupal_set_message(t('The new styles have been created'));
}

function breakpoints_admin_breakpoint_group_edit_form($form, &$form_state, $machine_name = '') {
  $form = array();
  $group = breakpoints_breakpoint_group_load($machine_name);
  $breakpoints = breakpoints_breakpoint_load_all();
  if (empty($breakpoints)) {
    return breakpoints_admin_breakpoint_group_edit_form_no_breakpoints();
  }

  $form_state['#breakpoint_group'] = $group;
  $is_new = $machine_name == '';
  $form_state['#is_new'] = $is_new;

  $form['name'] = array(
    '#type' => 'textfield',
    '#size' => '64',
    '#title' => t('group name'),
    '#required' => TRUE,
    '#default_value' => isset($group->name) ? $group->name : '',
    '#disabled' => !$is_new,
  );

  $form['machine_name'] = array(
    '#type' => 'machine_name',
    '#size' => '64',
    '#title' => t('Machine name'),
    '#required' => TRUE,
    '#default_value' => isset($group->machine_name) ? $group->machine_name : '',
    '#disabled' => !$is_new,
    '#machine_name' => array(
      'exists' => 'breakpoints_breakpoint_group_name_exists',
    ),
  );

  foreach ($breakpoints as $breakpoint_name => $breakpoint) {
    $options[$breakpoint_name] = $breakpoint->name . ' [' . $breakpoint->breakpoint . ']';
  }

  $form['breakpoints'] = array(
    '#title' => 'Select the breakpoints you want to use in this group',
    '#type' => 'checkboxes',
    '#options' => $options,
    '#default_value' => isset($group->breakpoints) ? drupal_map_assoc($group->breakpoints) : array(),
    '#required' => TRUE,
  );

  // Buttons
  $form['buttons'] = array(
    '#type' => 'container',
  );

  // Submit button
  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  if (!$is_new && $group->type == BREAKPOINTS_SOURCE_TYPE_CUSTOM) {
    $form['buttons']['deletelink'] = array(
      '#type' => 'markup',
      '#markup' => l(t('Delete this group'), 'admin/config/media/breakpoints/groups/' . $group->machine_name . '/delete', array(
        'query' => drupal_get_destination(),
      )),
    );
  }

  return $form;
}

function breakpoints_admin_breakpoint_group_edit_form_no_breakpoints() {
  $form = array();

  $form['info'] = array(
    '#type' => 'markup',
    '#markup' => t("There're no breakpoints defined, you'll have to create them first."),
  );

  return $form;
}

function breakpoints_admin_breakpoint_group_edit_form_validate($form, &$form_state) {
  $name = $form_state['values']['machine_name'];
  $label = $form_state['values']['name'];
}

function breakpoints_admin_breakpoint_group_edit_form_submit($form, &$form_state) {
  $machine_name = $form_state['values']['machine_name'];
  $name = $form_state['values']['name'];
  $breakpoints = array();
  foreach ($form_state['values']['breakpoints'] as $breakpoint => $status) {
    if ($status) {
      $breakpoints[] = $breakpoint;
    }
  }
  $is_new = $form_state['#is_new'];

  if ($is_new) {
    $new_group = breakpoints_breakpoint_group_empty_object();
    $new_group->machine_name = $machine_name;
    $new_group->name = $name;
    $new_group->type = BREAKPOINTS_SOURCE_TYPE_CUSTOM;
    $new_group->breakpoints = $breakpoints;
    breakpoints_breakpoint_group_save($new_group);
    menu_rebuild();
    $form_state['redirect'] = 'admin/config/media/breakpoints/groups/' . $machine_name;
    drupal_set_message(t('The new group have been created'));
  }
  else {
    $existing_group = breakpoints_breakpoint_group_load($machine_name);
    $existing_group->breakpoints = $breakpoints;
    breakpoints_breakpoint_group_save($existing_group);
  }
}

/**
 * Delete a group.
 */
function breakpoints_admin_breakpoint_group_delete_form($form, &$form_state, $machine_name) {
  $group = breakpoints_breakpoint_group_load($machine_name);
  $form_state['machine_name'] = $machine_name;
  $question = t('Are you sure you want to delete %group', array(
        '%group' => $group->name,
      ));
  $path = 'admin/config/media/breakpoints/groups';
  return confirm_form($form, $question, $path, '');
}

/**
 * Delete a group.
 */
function breakpoints_admin_breakpoint_group_delete_form_submit($form, &$form_state) {
  $machine_name = $form_state['machine_name'];
  breakpoints_breakpoint_group_delete_by_name($machine_name);
  menu_rebuild();
  $form_state['redirect'] = 'admin/config/media/breakpoints';
}

/**
 * Export a group.
 */
function breakpoints_admin_breakpoint_group_export_form($form, &$form_state, $machine_name) {
  // Create the export code textarea.
  ctools_include('export');
  $export = "/**\n * Breakpoints.\n */\n" . '$breakpoints = array();' . "\n";
  $group = breakpoints_breakpoint_group_load($machine_name);
  foreach ($group->breakpoints as $breakpoint) {
    if (!is_array($breakpoint) && !is_object($breakpoint)) {
      $breakpoint = breakpoints_breakpoint_load_by_fullkey($breakpoint);
    }
    $export .= ctools_export_object('breakpoints', $breakpoint) . "\n";
    $export .= '$breakpoints[] = $breakpoint;' . "\n\n";
  }
  $export .= "/**\n * Breakpoint group.\n */\n";
  $export .= ctools_export_object('breakpoint_group', $group);

  $form['export'] = array(
    '#type' => 'textarea',
    '#title' => t('Breakpoint group code'),
    '#rows' => 20,
    '#default_value' => $export,
  );
  return $form;
}

/**
 * Import a breakpoint group.
 */
function breakpoints_admin_breakpoint_group_import_form($form, &$form_state) {
  $form['import'] = array(
    '#type' => 'textarea',
    '#rows' => 10,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Import')
  );
  return $form;
}

/**
 * Validate a breakpoint group import.
 */
function breakpoints_admin_breakpoint_group_import_form_validate($form, &$form_state) {
  ctools_include('export');
  $code = $form_state['values']['import'];
  $group = ctools_export_crud_import('breakpoint_group', $code);
  if (!breakpoints_breakpoint_group_validate($group)) {
    form_set_error('import', t('Not a valid group object'));
    return;
  }
  if (breakpoints_breakpoint_group_name_exists($group->machine_name)) {
    form_set_error('import', t('A group with machine name %name already exists', array('%name' => $group->machine_name)));
    return;
  }
  foreach ($group->breakpoints as $key => $breakpoint) {
    // check if the breakpoint is a fully loaded object.
    if (is_array($breakpoint) || is_object($breakpoint)) {
      if (!breakpoints_breakpoint_validate($breakpoint)) {
        form_set_error('import',  t('The breakpoint group contains an invalid breakpoint.'));
        return;
      }
    }
  }
  // Manually imported groups are the same as custom made groups.
  $group->type = BREAKPOINTS_SOURCE_TYPE_CUSTOM;
  form_set_value($form['import'], $group, $form_state);
}

/**
 * Import breakpoint group.
 */
function breakpoints_admin_breakpoint_group_import_form_submit($form, &$form_state) {
  $group = $form_state['values']['import'];

  foreach ($group->breakpoints as $key => $breakpoint) {
    // check if the breakpoint is a fully loaded object.
    if (is_array($breakpoint) || is_object($breakpoint)) {
      $breakpoint = (object)$breakpoint;
      // If the breakpoints exist, only overwrite the custom ones.
      if ($existing_breakpoint = breakpoints_breakpoint_load_by_fullkey($breakpoint->machine_name)) {
        if ($breakpoint->source_type == BREAKPOINTS_SOURCE_TYPE_CUSTOM) {
          $breakpoint = (object)array_merge((array)$existing_breakpoint, (array)$breakpoint);
          breakpoints_breakpoint_save($breakpoint);
        }
      }
      else {
        breakpoints_breakpoint_save($breakpoint);
      }
      $group->breakpoints[$key] = $breakpoint->machine_name;
    }
  }
  if (breakpoints_breakpoint_group_save($group)) {
    drupal_set_message(t('Group %group saved.', array('%group' => $group->name)));
    $form_state['redirect'] = 'admin/config/media/breakpoints/groups/' . $group->machine_name;
  }
  else {
    drupal_set_message(t('Something went wrong, we could not save the group', 'error'));
  }
}

/**
 * Export a breakpoint.
 */
function breakpoints_admin_breakpoint_export_form($form, $form_state, $fullkey) {
  // Create the export code textarea.
  ctools_include('export');
  $breakpoint = breakpoints_breakpoint_load_by_fullkey($fullkey);
  $export = ctools_export_object('breakpoints', $breakpoint);

  $form['export'] = array(
    '#type' => 'textarea',
    '#title' => t('Preset code'),
    '#rows' => 20,
    '#default_value' => $export,
  );
  return $form;
}

/**
 * Import breakpoint.
 */
function breakpoints_admin_breakpoint_import_form($form, $form_state) {
  $form['import'] = array(
    '#type' => 'textarea',
    '#rows' => 10,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Import')
  );
  return $form;
}

/**
 * Validate a breakpoint import.
 */
function breakpoints_admin_breakpoint_import_form_validate($form, &$form_state) {
  ctools_include('export');
  $code = $form_state['values']['import'];
  $breakpoint = ctools_export_crud_import('breakpoints', $code);
  if (!breakpoints_breakpoint_validate($breakpoint)) {
    form_set_error('import', t('Not a valid breakpoint object'));
  }
  else {
    if (breakpoints_breakpoint_machine_name_exists($breakpoint->machine_name)) {
      form_set_error('import', t('A breakpoint with machine name %name already exists', array('%name' => $breakpoint->machine_name)));
    }
    else {
      form_set_value($form['import'], $breakpoint, $form_state);
    }
  }
}

/**
 * Import breakpoint.
 */
function breakpoints_admin_breakpoint_import_form_submit($form, &$form_state) {
  $breakpoint = $form_state['values']['import'];
  if (breakpoints_breakpoint_save($breakpoint)) {
    drupal_set_message(t('Breakpoint %breakpoint saved.', array('%breakpoint' => $breakpoint->name)));
    $form_state['redirect'] = 'admin/config/media/breakpoints/';
  }
  else {
    drupal_set_message(t('Something went wrong, we could not save the breakpoint'), 'error');
  }
}