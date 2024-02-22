export ARGOS_INSTALL_PATH=$HOME/daremo/alt
export PKG_CONFIG_PATH=$ARGOS_INSTALL_PATH/argos3-dist/lib/pkgconfig
export ARGOS_PLUGIN_PATH=$ARGOS_INSTALL_PATH/argos3-dist/lib/argos3
export LD_LIBRARY_PATH=$ARGOS_PLUGIN_PATH:$LD_LIBRARY_PATH
export PATH=$ARGOS_INSTALL_PATH/argos3-dist/bin/:$PATH
source /opt/ros/noetic/setup.bash
echo ok 
