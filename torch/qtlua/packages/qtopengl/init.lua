
require 'qt'
require 'qtcore'

if qt and qt.qApp and qt.qApp:runsWithoutGraphics() then
   print("qlua: not loading module qtopengl (running with -nographics)")
   return
end

require 'qtgui'
qt.require 'libqtopengl'

