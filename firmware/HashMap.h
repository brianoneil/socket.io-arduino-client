/*
	Based on HASHMAP Library by Alexander Brevig
	|| @file HashMap.h
	|| @version 1.0 Beta
	|| @author Alexander Brevig
	|| @contact alexanderbrevig@gmail.com

	@license
	|| | This library is free software; you can redistribute it and/or
	|| | modify it under the terms of the GNU Lesser General Public
	|| | License as published by the Free Software Foundation; version
	|| | 2.1 of the License.
	|| |
	|| | This library is distributed in the hope that it will be useful,
	|| | but WITHOUT ANY WARRANTY; without even the implied warranty of
	|| | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	|| | Lesser General Public License for more details.
	|| |
	|| | You should have received a copy of the GNU Lesser General Public
	|| | License along with this library; if not, write to the Free Software
	|| | Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
	|| #	
*/

#ifndef HASHMAP_H
#define HASHMAP_H

/* Handle association */
template<typename hash,typename map>
class HashType {
	public:
		HashType(){ reset(); }
		HashType(hash code,map value):hashCode(code),mappedValue(value){}
		void reset(){ hashCode = 0; mappedValue = 0; }
		hash getHash(){ return hashCode; }
		void setHash(hash code){ hashCode = code; }
		map getValue(){ return mappedValue; }
		void setValue(map value){ mappedValue = value; }		
		HashType& operator()(hash code, map value){
			setHash( code );
			setValue( value );
		}
	private:
		hash hashCode;
		map mappedValue;
};

template<typename hash,typename map>
class HashMap {
	public:
		HashMap(HashType<hash,map>* newMap,int newSize){
			hashMap = newMap;
			size = newSize;
			for (int i=0; i<size; i++){
				hashMap[i].reset();
			}
		}
		
		HashType<hash,map>& operator[](int x){
			//TODO - bounds
			return hashMap[x];
		}

		/*
			Added by @dantaex
			Specially for SocketIOClient
			Returns true if specified key exists, and
			sends back the stored function pointer.
		*/
		bool getFunction(hash key, void (**handlerp)( EthernetClient client, char *data  ) ){
			for (int i=0; i<size; i++){
				if (	strcmp( (char*)hashMap[i].getHash(), (char*)key )	== 0	){
					*handlerp = hashMap[i].getValue();
					return true;
				}
			}
			return false;
		}

	private:
		HashType<hash,map>* hashMap;
		int size;
};

#endif