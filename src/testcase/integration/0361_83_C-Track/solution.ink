// Translated from solution.cpp.

var maxn = 55;

var n: dynamic;

var m: dynamic;

var k: dynamic;

var ha = cpp_array(maxn, maxn);

var mat = cpp_array(maxn);

var stran = [1, 0, -1, 0, 0, 1, 0, -1];

var br: dynamic;

var bc: dynamic;

var er: dynamic;

var ec: dynamic;

func dis(r1: dynamic, c1: dynamic, r2: dynamic, c2: dynamic)
{
  return (abs((r1 - r2)) + abs((c1 - c2)));
}

class node
{
  var r: dynamic;
  var c: dynamic;
  var s: dynamic;
  var bu: dynamic;
  var cu: dynamic;
  var used: dynamic;
}

var que: dynamic;

var uu: dynamic;

func main()
{
  read(n, m, k);
  {
    var i = 0;
    while ((i < n))
    {
      read(mat[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < m))
        {
          if ((mat[i][j] == cpp_char("S")))
          {
            br = i;
            bc = j;
          }
          if ((mat[i][j] == cpp_char("T")))
          {
            er = i;
            ec = j;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  var now: dynamic;
  var ne: dynamic;
  now.bu = 0;
  now.s.clear();
  now.r = br;
  now.c = bc;
  now.cu = 0;
  now.used.clear();
  que.push(now);
  while ((!que.empty()))
  {
    now = que.top();
    que.pop();
    if ((mat[now.r][now.c] == cpp_char("T")))
    {
      write(now.s, "\n");
      return 0;
    }
    var ss = now.used;
    var ness: dynamic;
    var ro = now.s;
    var nr = now.r;
    var nc = now.c;
    uu.clear();
    {
      var i = 0;
      while ((i < ss.length()))
      {
        uu.insert((ss[i] - cpp_char("a")));
        i += 1;
      }
    }
    if ((ha[nr][nc].find(uu) != ha[nr][nc].end()))
    {
      continue;
    }
    ha[nr][nc].insert(uu);
    var ner: dynamic;
    var nec: dynamic;
    {
      var i = 0;
      while ((i < 4))
      {
        ner = (nr + stran[i][0]);
        nec = (nc + stran[i][1]);
        if (((((ner >= 0) && (ner < n)) && (nec >= 0)) && (nec < m)))
        {
          var p = mat[ner][nec];
          ne.r = ner;
          ne.c = nec;
          ne.bu = (now.bu + 1);
          if (((p != cpp_char("T")) && (p != cpp_char("S"))))
          {
            var hu = 0;
            {
              var j = 0;
              while ((j < ss.length()))
              {
                if ((p == ss[j]))
                {
                  hu = 1;
                  break;
                }
                j += 1;
              }
            }
            if ((hu == 1))
            {
              ne.used = ss;
              ne.cu = now.cu;
              ne.s = (now.s + p);
              que.push(ne);
            } else
            {
              if (((now.cu + 1) <= k))
              {
                ne.used = (ss + p);
                ne.cu = (now.cu + 1);
                ne.s = (now.s + p);
                que.push(ne);
              }
            }
          } else if ((p == cpp_char("T")))
          {
            ne = now;
            ne.bu += 1;
            ne.r = er;
            ne.c = ec;
            que.push(ne);
          }
        }
        i += 1;
      }
    }
  }
  write("-1", "\n");
  return 0;
}
