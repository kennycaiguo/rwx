
RSpec.describe WX::Color do
  subject { ::WX::Color::RED }
  
  include_examples "copyable", true
  include_examples "dumpable", true

  include_examples "struct_equal", {:red => 255, :green => 0, :blue => 0}
  
  describe "with subclass" do
    subject { class SubColor < WX::Color; end; SubColor.new(255, 0, 0) }
    include_examples "copyable", true
    include_examples "dumpable", true

    include_examples "struct_equal", {:red => 255, :green => 0, :blue => 0}
  end

  it "overflow on bounderies" do
    c = WX::Color.new(40, 0, 0)
    
    expect(c.alpha).to eq(255)
    
    expect { c.red = 300 }.to change { c.red }.
      from( 40 ).
      to( 44 )
    expect { c.blue += 256 }.not_to change { c.blue }
    
    expect { c.green = -26 }.to change { c.green }.
      from( 0 ).
      to( 230 )
    
    expect { c.alpha = true }.to change { c.alpha }.
      from( 255 ).
      to( 0 )
  end
  
#  it "not equal to color name before app-init" do
#    c = WX::Color.new(255, 0, 0)
#    expect(c.name).not_to eq("red")
#    expect(c.name).to be_nil
#  #  expect(c).not_to eq("red")
#  end
#
#  it "undefined color constants before app-init" do
#    expect { WX::Color::RED }.to raise_error(NameError)
#    expect { WX::Color::BLUE }.to raise_error(NameError)
#  end
  
  it "equal to color name after app-init" do
    c1 = WX::Color.new(255, 0, 0)
    expect(c1.name).to eq("red")
    expect(c1).to eq("red")
    expect(c1).to eq(WX::Color::RED)

    c2 = WX::Color.new(254, 0, 0)
    expect(c2.name).not_to eq("red")
    expect(c2.name).to be_nil
    expect(c2).not_to eq("red")
  end

  it "set color name" do
    c = WX::Color.new(254, 0, 254)
    expect(c.name).not_to eq("red")
    expect(c.name).to be_nil

    c.name = "foobar"

    expect(c.name).not_to eq("red")
    expect(c.name).not_to be_nil

    expect(c.name).to eq("foobar")
    expect(c).to eq(WX::Color["foobar"])
  end

  it "defined color constants after app-init" do
    expect { WX::Color::RED }.to_not raise_error
    expect { WX::Color::BLUE }.to_not raise_error
    expect { WX::Color::NOTHING }.to raise_error(NameError)
  end

end
