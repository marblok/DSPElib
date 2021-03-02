<!-- Tufts VUE 2.2.8 concept-map (DSPu_MorseKey_state_graph.vue) 2008-11-15 -->
<!-- Tufts VUE: http://vue.tufts.edu/ -->
<!-- Do Not Remove: VUE mapping @version(1.1) jar:file:/C:/Program%20Files/VUE/VUE.jar!/tufts/vue/resources/lw_mapping_1_1.xml -->
<!-- Do Not Remove: Saved date Sat Nov 15 16:37:00 CET 2008 by Marek on platform Windows XP 5.1 in JVM 1.5.0_11-b03 -->
<!-- Do Not Remove: Saving version @(#)VUE: built July 2 2008 at 1658 by vue on Linux 2.4.21-53.EL i386 JVM 1.5.0_06-b05 -->
<?xml version="1.0" encoding="US-ASCII"?>
<LW-MAP xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    xsi:noNamespaceSchemaLocation="none" ID="0"
    label="DSPu_MorseKey_state_graph.vue" x="0.0" y="0.0"
    width="1.4E-45" height="1.4E-45" strokeWidth="0.0" autoSized="false">
    <fillColor>#FFFFFF</fillColor>
    <strokeColor>#404040</strokeColor>
    <textColor>#000000</textColor>
    <font>SansSerif-plain-14</font>
    <metadata-list category-list-size="1" other-list-size="0"
        ontology-list-size="0" RCategoryListSize="0">
        <ontology-list-string></ontology-list-string>
        <metadata xsi:type="vue-metadata-element">
            <value></value>
            <key>http://vue.tufts.edu/vue.rdfs#none</key>
            <type>1</type>
        </metadata>
    </metadata-list>
    <URIString>http://vue.tufts.edu/rdf/resource/a08ff140c0a8006401ad186fa96867f0</URIString>
    <child ID="2"
        label="Morse State 0&#xa;end of char&#xa;expected_value = 0.0"
        x="411.0" y="176.0" width="126.0" height="53.0"
        strokeWidth="2.0" autoSized="true" xsi:type="node">
        <fillColor>#E6F7FD</fillColor>
        <strokeColor>#776D6D</strokeColor>
        <textColor>#000000</textColor>
        <font>Arial-plain-12</font>
        <metadata-list category-list-size="1" other-list-size="0"
            ontology-list-size="0" RCategoryListSize="0">
            <ontology-list-string></ontology-list-string>
            <metadata xsi:type="vue-metadata-element">
                <value></value>
                <key>http://vue.tufts.edu/vue.rdfs#none</key>
                <type>1</type>
            </metadata>
        </metadata-list>
        <URIString>http://vue.tufts.edu/rdf/resource/a08ff140c0a8006401ad186f2cd892f8</URIString>
        <shape arcwidth="20.0" archeight="20.0" xsi:type="roundRect"/>
    </child>
    <child ID="3" label="Algorithm start&#xa;value = 0.0" x="431.0"
        y="54.0" width="90.0" height="43.0" strokeWidth="2.0"
        autoSized="false" xsi:type="node">
        <fillColor>#FFFFFF</fillColor>
        <strokeColor>#776D6D</strokeColor>
        <textColor>#000000</textColor>
        <font>Arial-plain-12</font>
        <metadata-list category-list-size="1" other-list-size="0"
            ontology-list-size="0" RCategoryListSize="0">
            <ontology-list-string></ontology-list-string>
            <metadata xsi:type="vue-metadata-element">
                <value></value>
                <key>http://vue.tufts.edu/vue.rdfs#none</key>
                <type>1</type>
            </metadata>
        </metadata-list>
        <URIString>http://vue.tufts.edu/rdf/resource/a08ff140c0a8006401ad186f64627338</URIString>
        <shape arcwidth="20.0" archeight="20.0" xsi:type="roundRect"/>
    </child>
    <child ID="4" x="473.91733" y="96.5" width="2.2440796" height="80.0"
        strokeWidth="1.0" autoSized="false" controlCount="0"
        arrowState="2" xsi:type="link">
        <strokeColor>#404040</strokeColor>
        <textColor>#404040</textColor>
        <font>Arial-plain-11</font>
        <metadata-list category-list-size="1" other-list-size="0"
            ontology-list-size="0" RCategoryListSize="0">
            <ontology-list-string></ontology-list-string>
            <metadata xsi:type="vue-metadata-element">
                <value></value>
                <key>http://vue.tufts.edu/vue.rdfs#none</key>
                <type>1</type>
            </metadata>
        </metadata-list>
        <URIString>http://vue.tufts.edu/rdf/resource/a08ff140c0a8006401ad186fb21933c9</URIString>
        <point1 x="475.6614" y="97.0"/>
        <point2 x="474.41733" y="176.0"/>
        <ID1 xsi:type="node">3</ID1>
        <ID2 xsi:type="node">2</ID2>
    </child>
    <child ID="9"
        label="ON_counter = 0&#xa;OFF_counter = dash_len - dot_len"
        x="749.0" y="170.0" width="198.0" height="38.0"
        strokeWidth="2.0" autoSized="true" xsi:type="node">
        <fillColor>#FFFFFF</fillColor>
        <strokeColor>#776D6D</strokeColor>
        <textColor>#000000</textColor>
        <font>Arial-plain-12</font>
        <metadata-list category-list-size="1" other-list-size="0"
            ontology-list-size="0" RCategoryListSize="0">
            <ontology-list-string></ontology-list-string>
            <metadata xsi:type="vue-metadata-element">
                <value></value>
                <key>http://vue.tufts.edu/vue.rdfs#none</key>
                <type>1</type>
            </metadata>
        </metadata-list>
        <URIString>http://vue.tufts.edu/rdf/resource/a0981f7dc0a8006401ad186fa4e4d7ba</URIString>
        <shape arcwidth="20.0" archeight="20.0" xsi:type="roundRect"/>
    </child>
    <child ID="10"
        label="Morse State 3&#xa;key OFF - end of char&#xa;OFF_counter--&#xa;expected_value = 0.0"
        x="926.5" y="302.5" width="126.0" height="68.0"
        strokeWidth="2.0" autoSized="true" xsi:type="node">
        <fillColor>#E6F7FD</fillColor>
        <strokeColor>#776D6D</strokeColor>
        <textColor>#000000</textColor>
        <font>Arial-plain-12</font>
        <metadata-list category-list-size="1" other-list-size="0"
            ontology-list-size="0" RCategoryListSize="0">
            <ontology-list-string></ontology-list-string>
            <metadata xsi:type="vue-metadata-element">
                <value></value>
                <key>http://vue.tufts.edu/vue.rdfs#none</key>
                <type>1</type>
            </metadata>
        </metadata-list>
        <URIString>http://vue.tufts.edu/rdf/resource/a0981f7dc0a8006401ad186f3e72ef16</URIString>
        <shape arcwidth="20.0" archeight="20.0" xsi:type="roundRect"/>
    </child>
    <child ID="11" x="865.72705" y="207.5" width="91.655945"
        height="95.5" strokeWidth="1.0" autoSized="false"
        controlCount="0" arrowState="2" xsi:type="link">
        <strokeColor>#404040</strokeColor>
        <textColor>#404040</textColor>
        <font>Arial-plain-11</font>
        <metadata-list category-list-size="1" other-list-size="0"
            ontology-list-size="0" RCategoryListSize="0">
            <ontology-list-string></ontology-list-string>
            <metadata xsi:type="vue-metadata-element">
                <value></value>
                <key>http://vue.tufts.edu/vue.rdfs#none</key>
                <type>1</type>
            </metadata>
        </metadata-list>
        <URIString>http://vue.tufts.edu/rdf/resource/a0981f8cc0a8006401ad186f189533c3</URIString>
        <point1 x="866.2271" y="208.0"/>
        <point2 x="956.88306" y="302.5"/>
        <ID1 xsi:type="node">9</ID1>
        <ID2 xsi:type="node">10</ID2>
    </child>
    <child ID="12"
        label="lcurrent_morse_segment &#xa;>=&#xa; (strlen(THIS->morse_text)"
        x="536.5" y="175.39972" width="213.0" height="42.0"
        strokeWidth="1.0" autoSized="false" controlCount="0"
        arrowState="2" xsi:type="link">
        <strokeColor>#404040</strokeColor>
        <textColor>#404040</textColor>
        <font>Arial-plain-11</font>
        <metadata-list category-list-size="1" other-list-size="0"
            ontology-list-size="0" RCategoryListSize="0">
            <ontology-list-string></ontology-list-string>
            <metadata xsi:type="vue-metadata-element">
                <value></value>
                <key>http://vue.tufts.edu/vue.rdfs#none</key>
                <type>1</type>
            </metadata>
        </metadata-list>
        <URIString>http://vue.tufts.edu/rdf/resource/a0981f8cc0a8006401ad186f9ba78f90</URIString>
        <point1 x="537.0" y="200.22592"/>
        <point2 x="749.0" y="192.57353"/>
        <ID1 xsi:type="node">2</ID1>
        <ID2 xsi:type="node">9</ID2>
    </child>
    <child ID="13"
        label="Morse State 1&#xa;Key ON&#xa;ON_counter--&#xa;expected_value = 1.0"
        x="119.5" y="392.5" width="126.0" height="68.0"
        strokeWidth="2.0" autoSized="true" xsi:type="node">
        <fillColor>#E6F7FD</fillColor>
        <strokeColor>#776D6D</strokeColor>
        <textColor>#000000</textColor>
        <font>Arial-plain-12</font>
        <metadata-list category-list-size="1" other-list-size="0"
            ontology-list-size="0" RCategoryListSize="0">
            <ontology-list-string></ontology-list-string>
            <metadata xsi:type="vue-metadata-element">
                <value></value>
                <key>http://vue.tufts.edu/vue.rdfs#none</key>
                <type>1</type>
            </metadata>
        </metadata-list>
        <URIString>http://vue.tufts.edu/rdf/resource/a0981f8cc0a8006401ad186f12fb7c6c</URIString>
        <shape arcwidth="20.0" archeight="20.0" xsi:type="roundRect"/>
    </child>
    <child ID="14"
        label="Morse State 2&#xa;key OFF&#xa;OFF_counter--&#xa;expected_value =  0.0"
        x="488.5" y="557.5" width="129.0" height="68.0"
        strokeWidth="2.0" autoSized="true" xsi:type="node">
        <fillColor>#E6F7FD</fillColor>
        <strokeColor>#776D6D</strokeColor>
        <textColor>#000000</textColor>
        <font>Arial-plain-12</font>
        <metadata-list category-list-size="1" other-list-size="0"
            ontology-list-size="0" RCategoryListSize="0">
            <ontology-list-string></ontology-list-string>
            <metadata xsi:type="vue-metadata-element">
                <value></value>
                <key>http://vue.tufts.edu/vue.rdfs#none</key>
                <type>1</type>
            </metadata>
        </metadata-list>
        <URIString>http://vue.tufts.edu/rdf/resource/a0981f8cc0a8006401ad186f3931010b</URIString>
        <shape arcwidth="20.0" archeight="20.0" xsi:type="roundRect"/>
    </child>
    <child ID="15"
        label="ON_counter = dot_len&#xa;OFF_counter = dash_len&#xa;current_morse_segment++&#xa;value = 1.0"
        x="176.0" y="246.5" width="168.0" height="68.0"
        strokeWidth="2.0" autoSized="true" xsi:type="node">
        <fillColor>#FFFFFF</fillColor>
        <strokeColor>#776D6D</strokeColor>
        <textColor>#000000</textColor>
        <font>Arial-bold-12</font>
        <metadata-list category-list-size="1" other-list-size="0"
            ontology-list-size="0" RCategoryListSize="0">
            <ontology-list-string></ontology-list-string>
            <metadata xsi:type="vue-metadata-element">
                <value></value>
                <key>http://vue.tufts.edu/vue.rdfs#none</key>
                <type>1</type>
            </metadata>
        </metadata-list>
        <URIString>http://vue.tufts.edu/rdf/resource/a0981f8cc0a8006401ad186ff9793928</URIString>
        <shape arcwidth="20.0" archeight="20.0" xsi:type="roundRect"/>
    </child>
    <child ID="16"
        label="ON_counter = dot_len&#xa;OFF_counter = dot_len&#xa;current_morse_segment++&#xa;value = 1.0"
        x="287.0" y="332.5" width="168.0" height="68.0"
        strokeWidth="2.0" autoSized="true" xsi:type="node">
        <fillColor>#FFFFFF</fillColor>
        <strokeColor>#776D6D</strokeColor>
        <textColor>#000000</textColor>
        <font>Arial-bold-12</font>
        <metadata-list category-list-size="1" other-list-size="0"
            ontology-list-size="0" RCategoryListSize="0">
            <ontology-list-string></ontology-list-string>
            <metadata xsi:type="vue-metadata-element">
                <value></value>
                <key>http://vue.tufts.edu/vue.rdfs#none</key>
                <type>1</type>
            </metadata>
        </metadata-list>
        <URIString>http://vue.tufts.edu/rdf/resource/a0981f9cc0a8006401ad186f30753330</URIString>
        <shape arcwidth="20.0" archeight="20.0" xsi:type="roundRect"/>
    </child>
    <child ID="17" label="DOT" x="341.39468" y="224.22568"
        width="72.12714" height="26.92485" strokeWidth="1.0"
        autoSized="false" controlCount="0" arrowState="2" xsi:type="link">
        <strokeColor>#404040</strokeColor>
        <textColor>#404040</textColor>
        <font>Arial-plain-11</font>
        <metadata-list category-list-size="1" other-list-size="0"
            ontology-list-size="0" RCategoryListSize="0">
            <ontology-list-string></ontology-list-string>
            <metadata xsi:type="vue-metadata-element">
                <value></value>
                <key>http://vue.tufts.edu/vue.rdfs#none</key>
                <type>1</type>
            </metadata>
        </metadata-list>
        <URIString>http://vue.tufts.edu/rdf/resource/a0981f9cc0a8006401ad186fb0989f62</URIString>
        <point1 x="413.02182" y="224.7257"/>
        <point2 x="341.89468" y="250.65054"/>
        <ID1 xsi:type="node">2</ID1>
        <ID2 xsi:type="node">15</ID2>
    </child>
    <child ID="18" label="DASH" x="391.85364" y="228.5" width="66.00302"
        height="104.5" strokeWidth="1.0" autoSized="false"
        controlCount="0" arrowState="2" xsi:type="link">
        <strokeColor>#404040</strokeColor>
        <textColor>#404040</textColor>
        <font>Arial-plain-11</font>
        <metadata-list category-list-size="1" other-list-size="0"
            ontology-list-size="0" RCategoryListSize="0">
            <ontology-list-string></ontology-list-string>
            <metadata xsi:type="vue-metadata-element">
                <value></value>
                <key>http://vue.tufts.edu/vue.rdfs#none</key>
                <type>1</type>
            </metadata>
        </metadata-list>
        <URIString>http://vue.tufts.edu/rdf/resource/a0981f9cc0a8006401ad186fe1d7861e</URIString>
        <point1 x="457.3567" y="229.0"/>
        <point2 x="392.35367" y="332.5"/>
        <ID1 xsi:type="node">2</ID1>
        <ID2 xsi:type="node">16</ID2>
    </child>
    <child ID="19" x="200.04794" y="314.0" width="42.404114"
        height="79.0" strokeWidth="1.0" autoSized="false"
        controlCount="0" arrowState="2" xsi:type="link">
        <strokeColor>#404040</strokeColor>
        <textColor>#404040</textColor>
        <font>Arial-plain-11</font>
        <metadata-list category-list-size="1" other-list-size="0"
            ontology-list-size="0" RCategoryListSize="0">
            <ontology-list-string></ontology-list-string>
            <metadata xsi:type="vue-metadata-element">
                <value></value>
                <key>http://vue.tufts.edu/vue.rdfs#none</key>
                <type>1</type>
            </metadata>
        </metadata-list>
        <URIString>http://vue.tufts.edu/rdf/resource/a0981f9cc0a8006401ad186fff1c2376</URIString>
        <point1 x="241.95206" y="314.5"/>
        <point2 x="200.54794" y="392.5"/>
        <ID1 xsi:type="node">15</ID1>
        <ID2 xsi:type="node">13</ID2>
    </child>
    <child ID="20" x="244.99998" y="392.57208" width="43.019318"
        height="14.374847" strokeWidth="1.0" autoSized="false"
        controlCount="0" arrowState="2" xsi:type="link">
        <strokeColor>#404040</strokeColor>
        <textColor>#404040</textColor>
        <font>Arial-plain-11</font>
        <metadata-list category-list-size="1" other-list-size="0"
            ontology-list-size="0" RCategoryListSize="0">
            <ontology-list-string></ontology-list-string>
            <metadata xsi:type="vue-metadata-element">
                <value></value>
                <key>http://vue.tufts.edu/vue.rdfs#none</key>
                <type>1</type>
            </metadata>
        </metadata-list>
        <URIString>http://vue.tufts.edu/rdf/resource/a0981facc0a8006401ad186f8093ab2d</URIString>
        <point1 x="287.51932" y="393.07208"/>
        <point2 x="245.5" y="406.44693"/>
        <ID1 xsi:type="node">16</ID1>
        <ID2 xsi:type="node">13</ID2>
    </child>
    <child ID="21"
        label="ON_counter = 0&#xa;OFF_counter = space_len - dot_len&#xa;current_morse_segment++"
        x="593.5" y="359.5" width="204.0" height="53.0"
        strokeWidth="2.0" autoSized="true" xsi:type="node">
        <fillColor>#FFFFFF</fillColor>
        <strokeColor>#776D6D</strokeColor>
        <textColor>#000000</textColor>
        <font>Arial-plain-12</font>
        <metadata-list category-list-size="1" other-list-size="0"
            ontology-list-size="0" RCategoryListSize="0">
            <ontology-list-string></ontology-list-string>
            <metadata xsi:type="vue-metadata-element">
                <value></value>
                <key>http://vue.tufts.edu/vue.rdfs#none</key>
                <type>1</type>
            </metadata>
        </metadata-list>
        <URIString>http://vue.tufts.edu/rdf/resource/a09aeaa3c0a8006401ad186fc1c6ef4b</URIString>
        <shape arcwidth="20.0" archeight="20.0" xsi:type="roundRect"/>
    </child>
    <child ID="22" label="WORD END" x="505.48773" y="228.5"
        width="158.5245" height="131.5" strokeWidth="1.0"
        autoSized="false" controlCount="0" arrowState="2" xsi:type="link">
        <strokeColor>#404040</strokeColor>
        <textColor>#404040</textColor>
        <font>Arial-plain-11</font>
        <metadata-list category-list-size="1" other-list-size="0"
            ontology-list-size="0" RCategoryListSize="0">
            <ontology-list-string></ontology-list-string>
            <metadata xsi:type="vue-metadata-element">
                <value></value>
                <key>http://vue.tufts.edu/vue.rdfs#none</key>
                <type>1</type>
            </metadata>
        </metadata-list>
        <URIString>http://vue.tufts.edu/rdf/resource/a09aeaa3c0a8006401ad186f300d5d26</URIString>
        <point1 x="505.98776" y="229.0"/>
        <point2 x="663.51227" y="359.5"/>
        <ID1 xsi:type="node">2</ID1>
        <ID2 xsi:type="node">21</ID2>
    </child>
    <child ID="23" x="576.07666" y="412.0" width="101.547424"
        height="146.0" strokeWidth="1.0" autoSized="false"
        controlCount="0" arrowState="2" xsi:type="link">
        <strokeColor>#404040</strokeColor>
        <textColor>#404040</textColor>
        <font>Arial-plain-11</font>
        <metadata-list category-list-size="1" other-list-size="0"
            ontology-list-size="0" RCategoryListSize="0">
            <ontology-list-string></ontology-list-string>
            <metadata xsi:type="vue-metadata-element">
                <value></value>
                <key>http://vue.tufts.edu/vue.rdfs#none</key>
                <type>1</type>
            </metadata>
        </metadata-list>
        <URIString>http://vue.tufts.edu/rdf/resource/a09aeab2c0a8006401ad186f6cefb3b2</URIString>
        <point1 x="677.1241" y="412.5"/>
        <point2 x="576.57666" y="557.5"/>
        <ID1 xsi:type="node">21</ID1>
        <ID2 xsi:type="node">14</ID2>
    </child>
    <child ID="24" label="expected_value = 0.0&#xa;value = 0.0"
        x="291.5" y="555.5" width="135.5" height="38.0"
        strokeWidth="2.0" autoSized="false" xsi:type="node">
        <fillColor>#FFFFFF</fillColor>
        <strokeColor>#776D6D</strokeColor>
        <textColor>#000000</textColor>
        <font>Arial-bold-12</font>
        <metadata-list category-list-size="1" other-list-size="0"
            ontology-list-size="0" RCategoryListSize="0">
            <ontology-list-string></ontology-list-string>
            <metadata xsi:type="vue-metadata-element">
                <value></value>
                <key>http://vue.tufts.edu/vue.rdfs#none</key>
                <type>1</type>
            </metadata>
        </metadata-list>
        <URIString>http://vue.tufts.edu/rdf/resource/a09aeab2c0a8006401ad186f8e6534c6</URIString>
        <shape arcwidth="20.0" archeight="20.0" xsi:type="roundRect"/>
    </child>
    <child ID="26" label="ON_counter &lt;= 0" x="222.6047" y="460.0"
        width="114.45441" height="96.0" strokeWidth="1.0"
        autoSized="false" controlCount="0" arrowState="2" xsi:type="link">
        <strokeColor>#404040</strokeColor>
        <textColor>#404040</textColor>
        <font>Arial-plain-11</font>
        <metadata-list category-list-size="1" other-list-size="0"
            ontology-list-size="0" RCategoryListSize="0">
            <ontology-list-string></ontology-list-string>
            <metadata xsi:type="vue-metadata-element">
                <value></value>
                <key>http://vue.tufts.edu/vue.rdfs#none</key>
                <type>1</type>
            </metadata>
        </metadata-list>
        <URIString>http://vue.tufts.edu/rdf/resource/a09c47e1c0a8006401ad186f8f9ae9b7</URIString>
        <point1 x="223.1047" y="460.5"/>
        <point2 x="336.5591" y="555.5"/>
        <ID1 xsi:type="node">13</ID1>
        <ID2 xsi:type="node">24</ID2>
    </child>
    <child ID="27" x="426.5" y="579.9445" width="62.5" height="6.396118"
        strokeWidth="1.0" autoSized="false" controlCount="0"
        arrowState="2" xsi:type="link">
        <strokeColor>#404040</strokeColor>
        <textColor>#404040</textColor>
        <font>Arial-plain-11</font>
        <metadata-list category-list-size="1" other-list-size="0"
            ontology-list-size="0" RCategoryListSize="0">
            <ontology-list-string></ontology-list-string>
            <metadata xsi:type="vue-metadata-element">
                <value></value>
                <key>http://vue.tufts.edu/vue.rdfs#none</key>
                <type>1</type>
            </metadata>
        </metadata-list>
        <URIString>http://vue.tufts.edu/rdf/resource/a09c47e1c0a8006401ad186fe6a03a00</URIString>
        <point1 x="427.0" y="580.4445"/>
        <point2 x="488.5" y="585.84064"/>
        <ID1 xsi:type="node">24</ID1>
        <ID2 xsi:type="node">14</ID2>
    </child>
    <child ID="28" label="OFF_counter &lt;= 0" x="468.7384" y="228.5"
        width="88.0" height="329.5" strokeWidth="1.0" autoSized="false"
        controlCount="0" arrowState="2" xsi:type="link">
        <strokeColor>#404040</strokeColor>
        <textColor>#404040</textColor>
        <font>Arial-plain-11</font>
        <metadata-list category-list-size="1" other-list-size="0"
            ontology-list-size="0" RCategoryListSize="0">
            <ontology-list-string></ontology-list-string>
            <metadata xsi:type="vue-metadata-element">
                <value></value>
                <key>http://vue.tufts.edu/vue.rdfs#none</key>
                <type>1</type>
            </metadata>
        </metadata-list>
        <URIString>http://vue.tufts.edu/rdf/resource/a0a7cd58c0a8006401ad186f3d58e20a</URIString>
        <point1 x="546.0951" y="557.5"/>
        <point2 x="479.38174" y="229.0"/>
        <ID1 xsi:type="node">14</ID1>
        <ID2 xsi:type="node">2</ID2>
    </child>
    <child ID="29" label="state = 0&#xa;transmit new char" x="666.0"
        y="254.0" width="109.0" height="38.0" strokeWidth="2.0"
        autoSized="true" xsi:type="node">
        <fillColor>#FFFFFF</fillColor>
        <strokeColor>#776D6D</strokeColor>
        <textColor>#000000</textColor>
        <font>Arial-plain-12</font>
        <metadata-list category-list-size="1" other-list-size="0"
            ontology-list-size="0" RCategoryListSize="0">
            <ontology-list-string></ontology-list-string>
            <metadata xsi:type="vue-metadata-element">
                <value></value>
                <key>http://vue.tufts.edu/vue.rdfs#none</key>
                <type>1</type>
            </metadata>
        </metadata-list>
        <URIString>http://vue.tufts.edu/rdf/resource/a0acc62bc0a8006401ad186fe6e91cce</URIString>
        <shape arcwidth="20.0" archeight="20.0" xsi:type="roundRect"/>
    </child>
    <child ID="30" label="OFF_counter &lt;= 0" x="773.7551" y="285.1894"
        width="153.24487" height="36.938873" strokeWidth="1.0"
        autoSized="false" controlCount="0" arrowState="2" xsi:type="link">
        <strokeColor>#404040</strokeColor>
        <textColor>#404040</textColor>
        <font>Arial-plain-11</font>
        <metadata-list category-list-size="1" other-list-size="0"
            ontology-list-size="0" RCategoryListSize="0">
            <ontology-list-string></ontology-list-string>
            <metadata xsi:type="vue-metadata-element">
                <value></value>
                <key>http://vue.tufts.edu/vue.rdfs#none</key>
                <type>1</type>
            </metadata>
        </metadata-list>
        <URIString>http://vue.tufts.edu/rdf/resource/a0acc62bc0a8006401ad186f1daf6d27</URIString>
        <point1 x="926.5" y="321.62827"/>
        <point2 x="774.2551" y="285.6894"/>
        <ID1 xsi:type="node">10</ID1>
        <ID2 xsi:type="node">29</ID2>
    </child>
    <child ID="31" x="536.2102" y="219.93536" width="132.47174"
        height="38.60144" strokeWidth="1.0" autoSized="false"
        controlCount="0" arrowState="2" xsi:type="link">
        <strokeColor>#404040</strokeColor>
        <textColor>#404040</textColor>
        <font>Arial-plain-11</font>
        <metadata-list category-list-size="1" other-list-size="0"
            ontology-list-size="0" RCategoryListSize="0">
            <ontology-list-string></ontology-list-string>
            <metadata xsi:type="vue-metadata-element">
                <value></value>
                <key>http://vue.tufts.edu/vue.rdfs#none</key>
                <type>1</type>
            </metadata>
        </metadata-list>
        <URIString>http://vue.tufts.edu/rdf/resource/a0acc62bc0a8006401ad186fb7454408</URIString>
        <point1 x="668.18195" y="258.0368"/>
        <point2 x="536.7102" y="220.43536"/>
        <ID1 xsi:type="node">29</ID1>
        <ID2 xsi:type="node">2</ID2>
    </child>
    <child ID="32"
        label="Morse State 4&#xa;transmission end&#xa;expected_value = 0.0"
        x="941.0" y="527.0" width="126.0" height="53.0"
        strokeWidth="2.0" autoSized="true" xsi:type="node">
        <fillColor>#E6F7FD</fillColor>
        <strokeColor>#776D6D</strokeColor>
        <textColor>#000000</textColor>
        <font>Arial-plain-12</font>
        <metadata-list category-list-size="1" other-list-size="0"
            ontology-list-size="0" RCategoryListSize="0">
            <ontology-list-string></ontology-list-string>
            <metadata xsi:type="vue-metadata-element">
                <value></value>
                <key>http://vue.tufts.edu/vue.rdfs#none</key>
                <type>1</type>
            </metadata>
        </metadata-list>
        <URIString>http://vue.tufts.edu/rdf/resource/a0acc62bc0a8006401ad186fb03e2dc6</URIString>
        <shape arcwidth="20.0" archeight="20.0" xsi:type="roundRect"/>
    </child>
    <child ID="33" label="State 0&#xa;get new char" x="797.0" y="465.0"
        width="81.0" height="38.0" strokeWidth="2.0" autoSized="true" xsi:type="node">
        <fillColor>#FCDBD9</fillColor>
        <strokeColor>#776D6D</strokeColor>
        <textColor>#000000</textColor>
        <font>Arial-plain-12</font>
        <metadata-list category-list-size="1" other-list-size="0"
            ontology-list-size="0" RCategoryListSize="0">
            <ontology-list-string></ontology-list-string>
            <metadata xsi:type="vue-metadata-element">
                <value></value>
                <key>http://vue.tufts.edu/vue.rdfs#none</key>
                <type>1</type>
            </metadata>
        </metadata-list>
        <URIString>http://vue.tufts.edu/rdf/resource/a0af70aac0a8006401ad186f2577156a</URIString>
        <shape arcwidth="20.0" archeight="20.0" xsi:type="roundRect"/>
    </child>
    <child ID="34" label="znak = 0" x="874.77527" y="499.26807"
        width="71.030945" height="30.232147" strokeWidth="1.0"
        autoSized="false" controlCount="0" arrowState="3" xsi:type="link">
        <strokeColor>#EA2218</strokeColor>
        <textColor>#404040</textColor>
        <font>Arial-plain-11</font>
        <metadata-list category-list-size="1" other-list-size="0"
            ontology-list-size="0" RCategoryListSize="0">
            <ontology-list-string></ontology-list-string>
            <metadata xsi:type="vue-metadata-element">
                <value></value>
                <key>http://vue.tufts.edu/vue.rdfs#none</key>
                <type>1</type>
            </metadata>
        </metadata-list>
        <URIString>http://vue.tufts.edu/rdf/resource/a0af70aac0a8006401ad186fe43eb967</URIString>
        <point1 x="875.2753" y="499.7681"/>
        <point2 x="945.3063" y="529.00024"/>
        <ID1 xsi:type="node">33</ID1>
        <ID2 xsi:type="node">32</ID2>
    </child>
    <child ID="35" x="730.5355" y="291.5" width="96.928955"
        height="174.0" strokeWidth="1.0" autoSized="false"
        controlCount="0" arrowState="2" xsi:type="link">
        <strokeColor>#EA2218</strokeColor>
        <textColor>#404040</textColor>
        <font>Arial-plain-11</font>
        <metadata-list category-list-size="1" other-list-size="0"
            ontology-list-size="0" RCategoryListSize="0">
            <ontology-list-string></ontology-list-string>
            <metadata xsi:type="vue-metadata-element">
                <value></value>
                <key>http://vue.tufts.edu/vue.rdfs#none</key>
                <type>1</type>
            </metadata>
        </metadata-list>
        <URIString>http://vue.tufts.edu/rdf/resource/a0b4ac05c0a8006401ad186f484ba49d</URIString>
        <point1 x="731.0355" y="292.0"/>
        <point2 x="826.9645" y="465.0"/>
        <ID1 xsi:type="node">29</ID1>
        <ID2 xsi:type="node">33</ID2>
    </child>
    <child ID="36" label="State 1&#xa;transmit current char" x="658.0"
        y="593.0" width="124.0" height="38.0" strokeWidth="2.0"
        autoSized="true" xsi:type="node">
        <fillColor>#FCDBD9</fillColor>
        <strokeColor>#776D6D</strokeColor>
        <textColor>#000000</textColor>
        <font>Arial-plain-12</font>
        <metadata-list category-list-size="1" other-list-size="0"
            ontology-list-size="0" RCategoryListSize="0">
            <ontology-list-string></ontology-list-string>
            <metadata xsi:type="vue-metadata-element">
                <value></value>
                <key>http://vue.tufts.edu/vue.rdfs#none</key>
                <type>1</type>
            </metadata>
        </metadata-list>
        <URIString>http://vue.tufts.edu/rdf/resource/a0b4ac05c0a8006401ad186f3c0c9d19</URIString>
        <shape arcwidth="20.0" archeight="20.0" xsi:type="roundRect"/>
    </child>
    <child ID="37" label="znak != 0" x="736.9414" y="502.5"
        width="83.61713" height="91.0" strokeWidth="1.0"
        autoSized="false" controlCount="0" arrowState="2" xsi:type="link">
        <strokeColor>#EA2218</strokeColor>
        <textColor>#404040</textColor>
        <font>Arial-plain-11</font>
        <metadata-list category-list-size="1" other-list-size="0"
            ontology-list-size="0" RCategoryListSize="0">
            <ontology-list-string></ontology-list-string>
            <metadata xsi:type="vue-metadata-element">
                <value></value>
                <key>http://vue.tufts.edu/vue.rdfs#none</key>
                <type>1</type>
            </metadata>
        </metadata-list>
        <URIString>http://vue.tufts.edu/rdf/resource/a0b4ac15c0a8006401ad186ffa8e6fb7</URIString>
        <point1 x="820.05853" y="503.0"/>
        <point2 x="737.4414" y="593.0"/>
        <ID1 xsi:type="node">33</ID1>
        <ID2 xsi:type="node">36</ID2>
    </child>
    <child ID="38" x="489.41943" y="228.5" width="219.6666"
        height="365.0" strokeWidth="1.0" autoSized="false"
        controlCount="0" arrowState="2" xsi:type="link">
        <strokeColor>#EA2218</strokeColor>
        <textColor>#404040</textColor>
        <font>Arial-plain-11</font>
        <metadata-list category-list-size="1" other-list-size="0"
            ontology-list-size="0" RCategoryListSize="0">
            <ontology-list-string></ontology-list-string>
            <metadata xsi:type="vue-metadata-element">
                <value></value>
                <key>http://vue.tufts.edu/vue.rdfs#none</key>
                <type>1</type>
            </metadata>
        </metadata-list>
        <URIString>http://vue.tufts.edu/rdf/resource/a0b4ac15c0a8006401ad186f5636ed35</URIString>
        <point1 x="708.58606" y="593.0"/>
        <point2 x="489.91946" y="229.0"/>
        <ID1 xsi:type="node">36</ID1>
        <ID2 xsi:type="node">2</ID2>
    </child>
    <userZoom>1.0</userZoom>
    <userOrigin x="-12.0" y="-12.0"/>
    <presentationBackground>#202020</presentationBackground>
    <PathwayList currentPathway="0" revealerIndex="-1">
        <pathway ID="1" label="Untitled Pathway" x="0.0" y="0.0"
            width="1.4E-45" height="1.4E-45" strokeWidth="0.0"
            autoSized="false" currentIndex="-1" locked="false" open="true">
            <strokeColor>#B3993333</strokeColor>
            <textColor>#000000</textColor>
            <font>SansSerif-plain-14</font>
            <metadata-list category-list-size="0" other-list-size="0"
                ontology-list-size="0" RCategoryListSize="0">
                <ontology-list-string></ontology-list-string>
            </metadata-list>
            <URIString>http://vue.tufts.edu/rdf/resource/a08ff14fc0a8006401ad186f4cb12048</URIString>
            <masterSlide ID="5" x="0.0" y="0.0" width="800.0"
                height="600.0" strokeWidth="0.0" autoSized="false">
                <fillColor>#000000</fillColor>
                <strokeColor>#404040</strokeColor>
                <textColor>#000000</textColor>
                <font>SansSerif-plain-14</font>
                <metadata-list category-list-size="0"
                    other-list-size="0" ontology-list-size="0" RCategoryListSize="0">
                    <ontology-list-string></ontology-list-string>
                </metadata-list>
                <URIString>http://vue.tufts.edu/rdf/resource/a08ff18ec0a8006401ad186f6a7cd74b</URIString>
                <titleStyle ID="6" label="Header" x="335.5" y="172.5"
                    width="129.0" height="55.0" strokeWidth="0.0"
                    autoSized="true" isStyle="true" xsi:type="node">
                    <strokeColor>#404040</strokeColor>
                    <textColor>#FFFFFF</textColor>
                    <font>Gill Sans-plain-36</font>
                    <metadata-list category-list-size="0"
                        other-list-size="0" ontology-list-size="0" RCategoryListSize="0">
                        <ontology-list-string></ontology-list-string>
                    </metadata-list>
                    <URIString>http://vue.tufts.edu/rdf/resource/a08ff18ec0a8006401ad186f87c7749b</URIString>
                    <shape xsi:type="rectangle"/>
                </titleStyle>
                <textStyle ID="7" label="Slide Text" x="346.5" y="281.5"
                    width="107.0" height="37.0" strokeWidth="0.0"
                    autoSized="true" isStyle="true" xsi:type="node">
                    <strokeColor>#404040</strokeColor>
                    <textColor>#FFFFFF</textColor>
                    <font>Gill Sans-plain-22</font>
                    <metadata-list category-list-size="0"
                        other-list-size="0" ontology-list-size="0" RCategoryListSize="0">
                        <ontology-list-string></ontology-list-string>
                    </metadata-list>
                    <URIString>http://vue.tufts.edu/rdf/resource/a08ff18ec0a8006401ad186f791de48d</URIString>
                    <shape xsi:type="rectangle"/>
                </textStyle>
                <linkStyle ID="8" label="Links" x="373.5" y="384.0"
                    width="53.0" height="32.0" strokeWidth="0.0"
                    autoSized="true" isStyle="true" xsi:type="node">
                    <strokeColor>#404040</strokeColor>
                    <textColor>#B3BFE3</textColor>
                    <font>Gill Sans-plain-18</font>
                    <metadata-list category-list-size="0"
                        other-list-size="0" ontology-list-size="0" RCategoryListSize="0">
                        <ontology-list-string></ontology-list-string>
                    </metadata-list>
                    <URIString>http://vue.tufts.edu/rdf/resource/a08ff18ec0a8006401ad186f52b6de4c</URIString>
                    <shape xsi:type="rectangle"/>
                </linkStyle>
            </masterSlide>
        </pathway>
    </PathwayList>
    <date>2008-11-15</date>
    <mapFilterModel/>
    <modelVersion>4</modelVersion>
    <saveLocation>D:\Eclipse3.4\workspace\DSP_lib\Doxydoc</saveLocation>
    <saveFile>D:\Eclipse3.4\workspace\DSP_lib\Doxydoc\DSPu_MorseKey_state_graph.vue</saveFile>
</LW-MAP>
