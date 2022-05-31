---
title: Upgrading from an Earlier Greenplum 6 Release 
---

The upgrade path supported for this release is Greenplum Database 6.x to a newer Greenplum Database 6.x release.

**Important:** Set the Greenplum Database timezone to a value that is compatible with your host systems. Setting the Greenplum Database timezone prevents Greenplum Database from selecting a timezone each time the cluster is restarted and sets the timezone for the Greenplum Database coordinator and segment instances. After you upgrade to this release and if you have not set a Greenplum Database timezone value, verify that the selected Greenplum Database timezone is acceptable for your deployment. See [Configuring Timezone and Localization Settings](localization.html) for more information.

## <a id="gpdb_prereq"></a>Prerequisites 

Before starting the upgrade process, perform the following checks.

-   Verify the health of the Greenplum Database host hardware, and verify that the hosts meet the requirements for running Greenplum Database. The Greenplum Database `gpcheckperf` utility can assist you in confirming the host requirements.

    **Note:** If you need to run the `gpcheckcat` utility, run it a few weeks before the upgrade during a maintenance period. If necessary, you can resolve any issues found by the utility before the scheduled upgrade.

    The utility is in `$GPHOME/bin`. Place Greenplum Database in restricted mode when you run the `gpcheckcat` utility. See the *Greenplum Database Utility Guide* for information about the `gpcheckcat` utility.

    If `gpcheckcat` reports catalog inconsistencies, you can run `gpcheckcat` with the `-g` option to generate SQL scripts to fix the inconsistencies.

    After you run the SQL scripts, run `gpcheckcat` again. You might need to repeat the process of running `gpcheckcat` and creating SQL scripts to ensure that there are no inconsistencies. Run the SQL scripts generated by `gpcheckcat` on a quiescent system. The utility might report false alerts if there is activity on the system.

    **Important:** VMware customers should ontact VMware Support if the `gpcheckcat` utility reports errors but does not generate a SQL script to fix the errors. Information for contacting VMware Support is at [https://tanzu.vmware.com/support](https://tanzu.vmware.com/support).

-   If you have configured the Greenplum Platform Extension Framework \(PXF\) in your previous Greenplum Database installation, you must stop the PXF service, and you might need to back up PXF configuration files before upgrading to a new version of Greenplum Database. Refer to [PXF Pre-Upgrade Actions](../pxf/upgrade_pxf_6x.html#pxfpre) for instructions.

    If you have not yet configured PXF, no action is necessary.

-   If you have configured and used the Tanzu Greenplum Streaming Server \(GPSS\) in your previous Tanzu Greenplum Database installation, you must stop any running GPSS jobs and service instances before you upgrade to a new version of Greenplum Database. Refer to [GPSS Pre-Upgrade Actions](https://docs.vmware.com/en/VMware-Tanzu-Greenplum-Streaming-Server/1.7/greenplum-streaming-server/GUID-upgrading-gpss.html#step1-gpss-pre-upgrade-actions) for instructions.

    If you do not plan to use GPSS, or you have not yet configured GPSS, no action is necessary.


**Parent topic:** [Upgrading to Greenplum 6](upgrade_intro.html)

## <a id="topic17"></a>Upgrading from 6.x to a Newer 6.x Release 

An upgrade from Greenplum Database 6.x to a newer 6.x release involves stopping Greenplum Database, updating the Greenplum Database software binaries, and restarting Greenplum Database. If you are using Greenplum Database extension packages there are additional requirements. See [Prerequisites](#gpdb_prereq) in the previous section.

1.  Log in to your Greenplum Database coordinator host as the Greenplum administrative user:

    ```
    $ su - gpadmin
    ```

2.  Perform a smart shutdown of your Greenplum Database 6.x system \(there can be no active connections to the database\). This example uses the `-a` option to disable confirmation prompts:

    ```
    $ gpstop -a
    ```

3.  Copy the new Greenplum Database software installation package to the `gpadmin` user's home directory on each coordinator, standby, and segment host.
4.  *If you used `yum` or `apt` to install Greenplum Database to the default location*, run these commands on each host to upgrade to the new software release.

    For RHEL/CentOS systems:

    ```
    $ sudo yum upgrade ./greenplum-db-<version>-<platform>.rpm
    ```

    For Ubuntu systems:

    ```
    # apt install ./greenplum-db-<version>-<platform>.deb
    ```

    The `yum` or `apt` command installs the new Greenplum Database software files into a version-specific directory under `/usr/local` and updates the symbolic link `/usr/local/greenplum-db` to point to the new installation directory.

5.  *If you used `rpm` to install Greenplum Database to a non-default location on RHEL/CentOS systems*, run `rpm` on each host to upgrade to the new software release and specify the same custom installation directory with the `--prefix` option. For example:

    ```
    $ sudo rpm -U ./greenplum-db-<version>-<platform>.rpm --prefix=<directory>
    ```

    The `rpm` command installs the new Greenplum Database software files into a version-specific directory under the `<directory>` you specify, and updates the symbolic link `<directory>/greenplum-db` to point to the new installation directory.

6.  Update the permissions for the new installation. For example, run this command as `root` to change the user and group of the installed files to `gpadmin`.

    ```
    $ sudo chown -R gpadmin:gpadmin /usr/local/greenplum*
    ```

7.  If needed, update the `greenplum_path.sh` file on the coordinator and standby coordinator hosts for use with your specific installation. These are some examples.
    -   If Greenplum Database uses LDAP authentication, edit the `greenplum_path.sh` file to add the line:

        ```
        export LDAPCONF=/etc/openldap/ldap.conf
        ```
    -   If Greenplum Database uses PL/Java, you might need to set or update the environment variables `JAVA_HOME` and `LD_LIBRARY_PATH` in `greenplum_path.sh`.
    **Note:** When comparing the previous and new `greenplum_path.sh` files, be aware that installing some Greenplum Database extensions also updates the `greenplum_path.sh` file. The `greenplum_path.sh` from the previous release might contain updates that were the result of installing those extensions.

8.  Edit the environment of the Greenplum Database superuser \(`gpadmin`\) and make sure you are sourcing the `greenplum_path.sh` file for the new installation. For example change the following line in the `.bashrc` or your chosen profile file:

    ```
    source /usr/local/greenplum-db-<current_version>/greenplum_path.sh
    ```

    to:

    ```
    source /usr/local/greenplum-db-<new_version>/greenplum_path.sh
    ```

    Or if you are sourcing a symbolic link \(`/usr/local/greenplum-db`\) in your profile files, update the link to point to the newly installed version. For example:

    ```
    $ sudo rm /usr/local/greenplum-db
    $ sudo ln -s /usr/local/greenplum-db-<new_version> /usr/local/greenplum-db
    ```

9.  Source the environment file you just edited. For example:

    ```
    $ source ~/.bashrc
    ```

10. After all segment hosts have been upgraded, log in as the `gpadmin` user and restart your Greenplum Database system:

    ```
    # su - gpadmin
    $ gpstart
    ```

11. For Tanzu Greenplum Database, use the `gppkg` utility to re-install Tanzu Greenplum Database extensions. If you were previously using any Tanzu Greenplum Database extensions such as pgcrypto, PL/R, PL/Java, or PostGIS, download the corresponding packages from [VMware Tanzu Network](https://network.pivotal.io/products/pivotal-gpdb), and install using this utility. See the extension documentation for details.

    Also copy any files that are used by the extensions \(such as JAR files, shared object files, and libraries\) from the previous version installation directory to the new version installation directory on the coordinator and segment host systems.

12. If you configured PXF in your previous Greenplum Database installation, you may need to install PXF in your new Greenplum installation, and you may be required to re-initialize or register the PXF service after you upgrade Greenplum Database. Refer to the [Step 2](../pxf/upgrade_pxf_6x.html#pxfup) PXF upgrade procedure for instructions.
13. For Tanzu Greenplum Database, if you configured GPSS in your previous installation, you may be required to perform some upgrade actions, and you must re-restart the GPSS service instances and jobs. Refer to [Step 2](https://docs.vmware.com/en/VMware-Tanzu-Greenplum-Streaming-Server/1.7/greenplum-streaming-server/GUID-upgrading-gpss.html#step2-upgrading-gpss) of the GPSS upgrade procedure for instructions.

After upgrading Greenplum Database, ensure that all features work as expected. For example, test that backup and restore perform as expected, and Greenplum Database features such as user-defined functions, and extensions such as MADlib and PostGIS perform as expected.

## <a id="topic_zbx_szy_kbb"></a>Troubleshooting a Failed Upgrade 

If you experience issues during the migration process and have active entitlements for Greenplum Database or Tanzu Greenplum Database that were purchased through VMware, contact VMware Support. Information for contacting VMware Support is at [https://tanzu.vmware.com/support](https://tanzu.vmware.com/support).

**Be prepared to provide the following information:**

-   A completed [Upgrade Procedure](#topic17)
-   Log output from `gpcheckcat` \(located in `~/gpAdminLogs`\)
