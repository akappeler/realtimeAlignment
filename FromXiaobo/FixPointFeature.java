package com.tcl.rideoapp.FaceDetection.service;

import java.awt.image.BufferedImage;
import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Date;

import javax.imageio.ImageIO;
import javax.swing.ImageIcon;
import javax.swing.JFrame;
import javax.swing.JLabel;

import org.opencv.core.Core;
import org.opencv.core.Mat;
import org.opencv.core.MatOfByte;
import org.opencv.core.Point;
import org.opencv.core.Rect;
import org.opencv.core.Scalar;
import org.opencv.highgui.Highgui;
import org.opencv.imgproc.Imgproc;

public class FixPointFeature {

	public float roiRation = 0.1f;
	public float radio = 0.1f;
	public int xFixPoint = 10;
	public int yFixPoint = 10;
	public int numberFixPoint = xFixPoint * yFixPoint;
	public static int ratioRange = 4;
	public int briefThreshold = 0;
	public int skip = 2;
	
	public static void showResult(Mat img) {

	    MatOfByte matOfByte = new MatOfByte();
	    Highgui.imencode(".jpg", img, matOfByte);
	    byte[] byteArray = matOfByte.toArray();
	    BufferedImage bufImage = null;
	    try {
	        InputStream in = new ByteArrayInputStream(byteArray);
	        bufImage = ImageIO.read(in);
	        JFrame frame = new JFrame();
	        frame.getContentPane().add(new JLabel(new ImageIcon(bufImage)));
	        frame.pack();
	        frame.setVisible(true);
	    } catch (Exception e) {
	        e.printStackTrace();
	    }
}
	
	public static int[] removeBlackRegion_revised(Mat img)
	{
		int threshold = 30;
		int range = ratioRange;
		int roi[] = new int[4];
		roi[0] = 0;
		roi[1] = 0;
		roi[2] = img.cols() - 1;
		roi[3] = img.rows() - 1;
		int i;
		for(i=0;i<img.cols()/range;i++)
		{
			double []re = getColAverageIntensity(img,i);
			if((re[0]==re[1] && re[1]==re[2] &&re[0]<=threshold) ||  (re[0]<threshold && re[1]<threshold && re[2]<threshold))
			{
				continue;
			}
			else
			{
				roi[0] = i;
				break;
			}
		}
		
		if(i==img.cols()/range)
		{
			roi[0] = -1;
			return roi;
		}
		
		
		for(i=0;i<img.rows()/range;i++)
		{
			double []re = getRowAverageIntensity(img,i);
			if((re[0]==re[1] && re[1]==re[2]&&re[0]<=threshold) ||  (re[0]<threshold && re[1]<threshold && re[2]<threshold))
			{
				continue;
			}
			else
			{
				roi[1] = i;
				break;
			}
		}
		
			
		for(i=img.cols()-1;i>0;i--)
		{
			double []re = getColAverageIntensity(img,i);
			if((re[0]==re[1] && re[1]==re[2]&&re[0]<=threshold) ||  (re[0]<threshold && re[1]<threshold && re[2]<threshold))
			{
				continue;
			}
			else
			{
				roi[2] = i;
				break;
			}
		}
		
		for(i=img.rows()-1;i>0;i--)
		{
			double []re = getRowAverageIntensity(img,i);
			if((re[0]==re[1] && re[1]==re[2]&&re[0]<=threshold) ||  (re[0]<threshold && re[1]<threshold && re[2]<threshold))
			{
				continue;
			}
			else
			{
				roi[3] = i;
				break;
			}
		}
		return roi;
		
	}
	
	public int hammingDis(int a)
	{
		int c = 0;
		while(a>0)
		{
			a &= (a-1);
			c++;
		}
		return c;
	}
	
	public int disSimilarity(byte[]A, byte[]B)
	{
		int dis = 0;
		for(int i=0;i<A.length;i++)
		{
			int a = A[i]^B[i];
			dis += hammingDis(a);
		}
		return dis;
	}
	
	public static double[] getColAverageIntensity(Mat image,int startCol)
	{
		double c1 , c2,c3;
		int ct = 0;
		c1 = c2 = c3 = 0f;
		int range = ratioRange;
		for(int j=0;j<image.rows()/range;j++)
		{
			double[] data = image.get(j, startCol);
			c1 += data[0];
			c2 += data[1];
			c3 += data[2];
			ct++;
		}
		c1 = c1 / ct;
		c2 = c2 / ct;
		c3 = c3 / ct;

		
		double []re = new double [3];
		re[0] = c1;
		re[1] = c2;
		re[2] = c3;
		return re;
			
	}
	public static double[] getRowAverageIntensity(Mat image,int startRow)
	{
		double c1,c2,c3;
		int ct = 0;
		c1 = c2 = c3 = 0f;
		int range = ratioRange;
		for(int j=0;j<image.cols()/range;j++)
		{

			double[] data = image.get(startRow, j);
			c1 += data[0];
			c2 += data[1];
			c3 += data[2];

			ct++;
		}
		c1 = c1 / ct;
		c2 = c2 / ct;
		c3 = c3 / ct;
			
	
		
		double []re = new double [3];
		re[0] = c1;
		re[1] = c2;
		re[2] = c3;
		return re;
		
	}
	public byte[] briefImageDes(int[] image)
	{
	
		StringBuffer feat = new StringBuffer ();
		for(int i=0;i<image.length;i+=skip)
		{
			for(int j=i+1;j<image.length;j+=skip)
			{
				if(image[j]-image[i]>briefThreshold)
					feat.append("1");
				else
					feat.append("0");
				
			}
	
		}
		String str = new String(feat);
	
		byte []p = fromBinary(str);
//		for(int i=0;i<p.length;i++)
//		{
//			System.out.print((int)p[i] + " ");
//		}
//		System.out.println();
//		System.out.println(p.length);

		return p;
	}
	
	public int[] fixPointFeatureExtraction(Mat img)
	{
//		Imgproc.cvtColor(img, img, Imgproc.COLOR_RGB2GRAY);		
		int rows = (int) (img.rows() * (1-roiRation*2));
		int cols = (int) (img.cols() * (1-roiRation*2));
		int []smallImage = new int[numberFixPoint]; 
		
		int recWidth  = (int) (cols * radio);
		int recHeight = (int) (rows * radio);
		
		int xStep = (int) cols / xFixPoint;
		int yStep = (int) rows / yFixPoint;
		
//		System.out.println("xStep\t" + xStep);
//		System.out.println("yStep\t" + yStep);
		
//		System.out.println("recWidth\t" + recWidth);
//		System.out.println("recHeight\t" + recHeight);
		
		int cc = 0;
		for(int i=0;i<yFixPoint;i++)
		{
			
			int startY = (int) (img.rows() * roiRation) + yStep*i;
			for(int j=0;j<xFixPoint;j++)
			{
				int startX = (int) (img.cols() * roiRation) + xStep*j;
	//			Core.circle(img, new Point(startX,startY), 10, new Scalar(255,255,255), 1);
				Point pt1 = new Point(startX,startY);
				Point pt2 = new Point(startX + recWidth,startY + recHeight);
				
	//			Core.rectangle(img, pt1, pt2, new Scalar(255,255,255),1);
				Rect roi = new Rect(startX,startY,recWidth,recHeight);
				Mat partImage = new Mat(img,roi);
			
				Scalar s = Core.mean(partImage);
				smallImage[cc++] = (int) s.val[0];
				//System.out.println();				
			}
		}
	//	showResult(img);	
		return smallImage;
	}
	
	
	
	public static byte[] longToBytes(long x) {
	    ByteBuffer buffer = ByteBuffer.allocate(Long.SIZE/8);
	    buffer.putLong(x);
	    return buffer.array();
	}
	public static long bytesToLong(byte[] bytes) {
	    ByteBuffer buffer = ByteBuffer.allocate(Long.SIZE/8);
	    buffer.put(bytes);
	    buffer.flip();//need flip 
	    return buffer.getLong();
	}
	
	
	public static void main(String []str)
	{
		Date d = new Date();
		System.out.println(d.getTime());
		byte[] yy = longToBytes(d.getTime());
		long yyout = bytesToLong(yy);
		
		
		int aa = -1;
		int bbb = -5;
		int ccc = aa ^ bbb;
		System.loadLibrary( Core.NATIVE_LIBRARY_NAME );
		FixPointFeature f = new FixPointFeature();
		String imagePath = "C:\\Users\\TCL\\Desktop\\t\\";
		File []files = new File(imagePath).listFiles();
	
//		String bb = "11101000011010011";
//		int len = bb.length();
//		System.out.println(len);
//		byte []p = f.fromBinary(bb);
//		String o = f.toBinary(p);
		
		ArrayList<byte[]> data = new ArrayList<byte[]>();
		for(int i=0;i<files.length;i++)
		{
			
			Mat imageInput = Highgui.imread(files[i].getAbsolutePath());
			Date be = new Date();
			int []roi = f.removeBlackRegion_revised(imageInput);
			if(roi[0]==-1)
				 continue;
			Mat imm = new  Mat (imageInput,new Rect(roi[0],roi[1],roi[2]-roi[0]+1,roi[3]-roi[1]+1));
			//showResult(imm);
			
			int []re = f.fixPointFeatureExtraction(imm);
			byte[] feat = f.briefImageDes(re);
			data.add(feat);
			Date e = new Date();
			System.out.println("Process time\t" + (e.getTime()-be.getTime()));
			
		}
		
		System.out.println(data.size());
		Date be = new Date();
		
		for(int i=0;i<data.size();i++)
		{
			int diff = f.disSimilarity(data.get(0), data.get(1));
			System.out.println("dis \t " + i + " " + diff);
		}
		Date end = new Date();
		System.out.println("Process time\t" + (end.getTime()-be.getTime()));
		
		
	}
	
	
		
	byte[] fromBinary( String s )
	{
	    int sLen = s.length();
	    byte[] toReturn = new byte[(sLen + Byte.SIZE - 1) / Byte.SIZE];
	    char c;
	    for( int i = 0; i < sLen; i++ )
	        if( (c = s.charAt(i)) == '1' )
	            toReturn[i / Byte.SIZE] = (byte) (toReturn[i / Byte.SIZE] | (0x80 >>> (i % Byte.SIZE)));
	        else if ( c != '0' )
	            throw new IllegalArgumentException();
	    return toReturn;
	}
	String toBinary( byte[] bytes )
	{
	    StringBuilder sb = new StringBuilder(bytes.length * Byte.SIZE);
	    for( int i = 0; i < Byte.SIZE * bytes.length; i++ )
	        sb.append((bytes[i / Byte.SIZE] << i % Byte.SIZE & 0x80) == 0 ? '0' : '1');
	    return sb.toString();
	}
}
