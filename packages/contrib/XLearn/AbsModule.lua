local AbsModule, parent = torch.class('nn.AbsModule', 'nn.Module')

function AbsModule:__init(args)
   parent.__init(self)
  
   -- usage
   if args then
      error(toolBox.usage('nn.AbsModule',
                          'a simple component-wise mapping: abs()',
                          'abs = nn.AbsModule()\n'..
                             'rectified = abs:forward(sometensor)',
                          {type='nil', help='no arg required'}))
   end
end

function AbsModule:write(file)
   parent.write(self, file)
end

function AbsModule:read(file)
   parent.read(self, file)
end
