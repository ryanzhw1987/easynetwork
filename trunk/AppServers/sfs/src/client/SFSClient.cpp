/*
 * SFSClient.cpp
 *
 *  Created on: 2012-11-8
 *      Author: LiuYongJin
 */
#include <string>
using std::string;
#include "slog.h"
#include "SFSFile.h"

int main(int agrc, char* argv[])
{
	SLOG_INIT(NULL);
	//get gapth
	string master_addr="127.0.0.1";
	int master_port = 3012;
	SFS::File sfs_file(master_addr, master_port, 2);

	/*
	//file info
	string fid="AAACCCDDD";
	SFS::FileInfo fileinfo;
	if(sfs_file.file_info(&fileinfo, fid))
	{
		SLOG_INFO("result:%d FID:%s FileSize:%lld.", fileinfo.result, fileinfo.fid.c_str(), fileinfo.size);
		vector<ChunkInfo>::iterator it;
		for(it=fileinfo.chunkinfo.begin(); it!=fileinfo.chunkinfo.end(); ++it)
			SLOG_INFO("ChunkInfo:ChunkPath:%s ChunkAdd:%s ChunkPort:%d.",it->path.c_str(), it->chunk_addr.c_str(), it->port);
	}
	*/

	string filename="/data/test.txt";
	sfs_file.store(filename);

	SLOG_UNINIT();
	return 0;
}


