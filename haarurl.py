import urllib2
import urllib
import cv2
import numpy as np
import os
def store_raw_images():
	neg_images_link = 'http://image-net.org/api/text/imagenet.synset.geturls?wnid=n04158807'
	neg_image_urls = urllib2.urlopen(neg_images_link).read().decode()

	if not os.path.exists('neg'):
		os.makedirs('neg')

	pic_num =  len(os.listdir('neg/')) + 1

	for i in neg_image_urls.split('\n'):
		try:
			print(i)
			urllib.urlretrieve(i, "neg/"+str(pic_num)+'.jpg')
			img = cv2.imread("neg/"+str(pic_num)+'.jpg', cv2.IMREAD_GRAYSCALE)
			resized_image = cv2.resize(img, (100,100))
			cv2.imwrite("neg/"+str(pic_num)+'.jpg', resized_image)
			pic_num+=1

		except Exception as e:
			print(str(e))

#store_raw_images()

def find_uglies():
	for file_type in ['neg']:
		for img in os.listdir(file_type):
			for ugly in os.listdir('uglies'):
				try:
					current_image_path = str(file_type)+'/'+str(img)
					ugly=cv2.imread('uglies/'+str(ugly))
					question = cv2.imread(current_image_path)

					if ugly.shape == question.shape and not(np.bitwise_xor(ugly,question).any()):
						print('Found an ugly')
						print(current_image_path)
						os.remove(current_image_path)

				except Exception as e:
					print(str(e))

find_uglies()
