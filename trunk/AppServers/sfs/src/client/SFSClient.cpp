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
	string fid="AAACCCDDD";
	string master_addr="127.0.0.1";
	int master_port = 3012;
	SFS::File sfs_file(master_addr, master_port, 2);

	SFS::FileInfo file_info;
	if(sfs_file.file_info(fid, file_info))
	{
		printf("result:%d\nFID:%s\nFileSize:%lld\n", file_info.result, file_info.fid.c_str(), file_info.size);
		vector<ChunkInfo>::iterator it;
		for(it=file_info.chunkinfo.begin(); it!=file_info.chunkinfo.end(); ++it)
			printf("ChunkInfo:\n\tChunkPath:%s\n\tChunkAdd:%s\n\tChunkPort:%d\n",it->path.c_str(), it->chunk_addr.c_str(), it->port);
	}

	SLOG_UNINIT();
	return 0;
}


