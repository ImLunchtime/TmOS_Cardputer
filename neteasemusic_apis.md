# 网易云音乐APIs （供参考）

## 搜索
调用此接口 , 传入搜索关键词可以搜索该音乐 / 专辑 / 歌手 / 歌单 / 用户 , 关键词可以多个 , 以空格隔开 , 如 " 周杰伦 搁浅 "( 不需要登录 )

必选参数 : keywords : 关键词

可选参数 :
 - limit : 返回数量 , 默认为 30 
 - offset : 偏移数量，用于分页。
 - type: 搜索类型；默认为 1 即单曲 , 取值意义 : 1: 单曲, 10: 专辑, 100: 歌手, 1000: 歌单, 1002: 用户, 1004: MV, 1006: 歌词, 1009: 电台, 1014: 视频, 1018:综合, 2000:声音(搜索声音返回字段格式会不一样)

接口地址 : http(-s)://apis.netstart.cn/music/search

调用例子 : http(-s)://apis.netstart.cn/music/search?keywords=海阔天空

接收数据：
```
{
  "result": {
    "songs": [
      {
        "album": {
          "publishTime": 1262275200000,
          "size": 40,
          "artist": {
            "img1v1Url": "https://p2.music.126.net/6y-UleORITEDbvrOLV0Q8A==/5639395138885805.jpg",
            "musicSize": 0,
            "albumSize": 0,
            "img1v1": 0,
            "name": "",
            "alias": [],
            "id": 0,
            "picId": 0
          },
          "copyrightId": 1416618,
          "name": "绕梁经典金曲",
          "id": 84786702,
          "picId": 109951164608584060,
          "mark": 0,
          "status": 1
        },
        "fee": 1,
        "duration": 262493,
        "rtype": 0,
        "ftype": 0,
        "artists": [
          {
            "img1v1Url": "https://p2.music.126.net/6y-UleORITEDbvrOLV0Q8A==/5639395138885805.jpg",
            "musicSize": 0,
            "albumSize": 0,
            "img1v1": 0,
            "name": "庄学忠",
            "alias": [],
            "id": 6656,
            "picId": 0
          }
        ],
        "copyrightId": 1416618,
        "mvid": 0,
        "name": "365里路",
        "alias": [],
        "id": 1414849939,
        "mark": 17179877376,
        "status": 0
      }
    ]
  }
}
```


## 获取歌曲下载地址
GET访问`music.163.com/song/media/outer/url?id=<歌曲ID>`
一般情况下将会被重定向到实际下载链接，但有可能会返回403/404错误，这意味着歌曲需要会员才可以下载或者ID是错误的。
