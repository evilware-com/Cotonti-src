/* r923 add columns and config option for new PFS system
ALTER TABLE sed_pfs_folders ADD pff_parentid INT(11) AFTER pff_id;
ALTER TABLE sed_pfs_folders ADD pff_path VARCHAR(255) AFTER pff_desc;
INSERT INTO `sed_config` (`config_owner`, `config_cat`, `config_order`, `config_name`, `config_type`, `config_value`, `config_default`, `config_text`) VALUES ('core', 'pfs', '06', 'flashupload', 3, '0', '', '');
*/