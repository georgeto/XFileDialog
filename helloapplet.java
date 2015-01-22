import java.awt.*;
import java.awt.event.*;
import java.applet.*;
import java.util.*;
import javax.swing.*;
import net.tomahawk.*;
import java.io.*;

public class helloapplet extends JApplet implements ActionListener {

    String title1= "Open File Dialog (single-selection)";
    String title2= "Open File Dialog (multi-selection)";
    String title3= "Open Folder Dialog (single-selection for folder)";
    String title4= "Open Folder Dialog (multi-selection for folders)";
    String title5 = "Open Thumbnail Dialog (single-selection)";
    String title6 = "Open Thumbnail Dialog (multi-selection)";
    String title7 = "Save File Dialog (single-selection)";
    String title8 = "Open the Old AWT FileDialog (for comparision)";

    private JTextField selectioncontent;


    public void actionPerformed(ActionEvent e) {

        String action = e.getActionCommand();

// get single file, return value is a relative filename;
        if(action.equals(title1)) {
            XFileDialog dlg=new XFileDialog(helloapplet.this);
            dlg.setTitle(title1);
            String content =dlg.getFile();
            String folder = dlg.getDirectory();
            selectioncontent.setText("filename:" + content);

            System.out.println("APP>>: current Dir: " + folder );

            if(content!=null) {
                File tempfile = new File(folder, content);
                if(tempfile.exists() ) {
                    System.out.println("APP>>: The file \"" + tempfile.getPath() + "\" does exist\n");
                } else {
                    System.out.println("APP>>: Error: The file \"" + tempfile.getPath() + "\" does not exist\n");
                }

            }

            dlg.dispose();

        }

// get multi-files, return values are relative names
        if(action.equals(title2) ) {
            XFileDialog dlg=new XFileDialog(helloapplet.this);
            dlg.setTitle(title2);
            String content =null;
            // multi-selection will be turned on automatically
            // when you use getFiles() or getFolders();
            String[] filenames = dlg.getFiles();

            if(filenames!=null && filenames.length>0) {
                content="";
                for(int i=0; i<filenames.length; i++) {
                    System.out.println("APP>>: returned filenames:  " + filenames[i]);
                    content+= filenames[i] + "; ";
                }

                System.out.println("APP>>: current dir: " + dlg.getDirectory() );

            }


            selectioncontent.setText("filenames:    " + content);
            dlg.dispose();

        }

// get a folder, the return value is an absolute (full) path name
        if(action.equals(title3) ) {
            XFileDialog dlg=new XFileDialog(helloapplet.this);
            dlg.setTitle(title3);

            String content = dlg.getFolder();

            selectioncontent.setText("foldername:  " + content);

            System.out.println("APP>>: current Dir: " + dlg.getDirectory() );

            dlg.dispose();

        }

// get folders, the return values are absolute (full) path names
        if(action.equals(title4) ) {
            XFileDialog dlg=new XFileDialog(helloapplet.this);
            dlg.setTitle(title4);
            String[] itemNames = dlg.getFolders();

            String content= null;
            if(itemNames!=null && itemNames.length > 0) {
                content="";
                for(int i=0; i<itemNames.length; i++)
                    content += itemNames[i] + ";";

            }

            selectioncontent.setText("foldernames:    " + content);

            System.out.println("APP>>: current Dir: " + dlg.getDirectory() );
            dlg.dispose();

        }

        // Show how to addFilters for the native dialog
        // Show how to turn on the thumbnail mode
        //
        //
        if(action.equals(title5) ) {
            XFileDialog dlg=new XFileDialog(helloapplet.this);
            dlg.setThumbnail(true);
            dlg.setTitle(title5);

            ArrayList<String> ext0= new ArrayList<String>();
            ext0.add("*");
            ExtensionsFilter filter0 = new ExtensionsFilter("All Files", ext0);

            ArrayList<String> ext1= new ArrayList<String>();
            ext1.add("jpg");
            ext1.add("jpeg");
            ExtensionsFilter filter1= new ExtensionsFilter("Jpg image", ext1) ;

            ArrayList<String> ext2= new ArrayList<String>();
            ext2.add("bmp");
            ExtensionsFilter filter2 = new ExtensionsFilter("Bmp image", ext2) ;

            ArrayList<String> ext3= new ArrayList<String>();
            ext3.add("png");
            ExtensionsFilter filter3 =  new ExtensionsFilter("Png image", ext3);

            dlg.addFilters(filter0, filter1, filter2, filter3);

            String filename = dlg.getFile();

            selectioncontent.setText("filename:    " + filename);
            System.out.println("APP>>: current Dir: " + dlg.getDirectory() );

            dlg.dispose();

        }


        // get multi-files in thumbnail view mode
        // Note: setThumbnail(true) force the dialog into thumbnail mode
        // in Windows XP/Vista/Win7
        //
        // But setThumbnail(false) does not mean disable thumbnail mode,
        // it means let the filedialog determine its own view mode.
        //
        // In Vista, its native filedialog (ifiledialog) can remember
        // user's viewmode when last visited, and use that mode as default.
        //
        //
        //
        if(action.equals(title6) ) {
            XFileDialog dlg=new XFileDialog(helloapplet.this);
            dlg.setThumbnail(true);
            dlg.setTitle(title6);

            String content =null;
            String[] filenames = dlg.getFiles();
            if(filenames!=null && filenames.length>0) {
                content="";
                for(int i=0; i<filenames.length; i++) {
                    System.out.println("APP>>: Returned filenames: " + filenames[i]);
                    content+= filenames[i] + "; ";
                }
            }

            selectioncontent.setText("filenames:    " + content);
            System.out.println("APP>>: current Dir: " + dlg.getDirectory() );

            dlg.dispose();

        }

        if(action.equals(title7)) { // test the save file dialog
            XFileDialog dlg=new XFileDialog(helloapplet.this);

            // Important: we do not implement the setMode() method,
            // instead, we use the caller function "getSaveFile()"
            // to tell XFileDialog to use "Save File Mode"
            //
            //

            dlg.setTitle(title7);
            String content =dlg.getSaveFile();
            String folder = dlg.getDirectory();
            selectioncontent.setText("filename:" + content);

            System.out.println("APP>>: current Dir: " + folder );

            dlg.dispose();

        }


        if(action.equals(title8) ) { // get one file with AWT filedialog
            FileDialog dlg=new FileDialog((Frame)null);
            dlg.setMode(FileDialog.LOAD);
            dlg.setTitle(title8);
            dlg.show();
            String content =null;
            String filename = dlg.getFile();

            selectioncontent.setText("filename:    " + filename);
            System.out.println("APP>>: current Dir: " + dlg.getDirectory() );

            dlg.dispose();

        }


    }

    public void init() {

        // set lookandfeel, I prefer nimbus or mcwin(jtattoo)
        try {
            UIManager.setLookAndFeel("com.sun.java.swing.plaf.nimbus.NimbusLookAndFeel");
//	UIManager.setLookAndFeel( UIManager.getSystemLookAndFeelClassName() );
        } catch(Exception e) {
            e.printStackTrace();
        }

        JPanel main = new JPanel(new BorderLayout());
        JButton button1 = new JButton(title1);
        JButton button2 = new JButton(title2);
        JButton button3 = new JButton(title3);
        JButton button4 = new JButton(title4);
        JButton button5 = new JButton(title5);
        JButton button6 = new JButton(title6);
        JButton button7 = new JButton(title7);
        JButton button8 = new JButton(title8);

        JLabel label1 = new JLabel("Selection:");
        selectioncontent = new JTextField();

        main.setLayout(new FlowLayout());

        label1.setPreferredSize(new Dimension(500, 30));
        selectioncontent.setPreferredSize(new Dimension(500, 30));

        button1.setPreferredSize(new Dimension(500, 36));
        button2.setPreferredSize(new Dimension(500, 36));
        button3.setPreferredSize(new Dimension(500, 36));
        button4.setPreferredSize(new Dimension(500, 36));
        button5.setPreferredSize(new Dimension(500, 36));
        button6.setPreferredSize(new Dimension(500, 36));
        button7.setPreferredSize(new Dimension(500, 36));
        button8.setPreferredSize(new Dimension(500, 36));


        main.add(label1);
        main.add(selectioncontent);
        main.add(button1);
        main.add(button2);
        main.add(button3);
        main.add(button4);
        main.add(button5);
        main.add(button6);
        main.add(button7);
        main.add(button8);


        getContentPane().add(main);
        button1.addActionListener(this);
        button2.addActionListener(this);
        button3.addActionListener(this);
        button4.addActionListener(this);
        button5.addActionListener(this);
        button6.addActionListener(this);
        button7.addActionListener(this);
        button8.addActionListener(this);

        validate();
    }


    public void start() {

    }

    public void stop() {
    }

    public void destroy() {

    }
}







