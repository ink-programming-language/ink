// Translated from solution.cpp.

var maxn: dynamic = 55;

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var ha: dynamic = cpp_array(maxn, maxn);

var mat: dynamic = cpp_array(maxn);

var stran: dynamic = [1, 0, -1, 0, 0, 1, 0, -1];

var br: dynamic = cpp_uninitialized();

var bc: dynamic = cpp_uninitialized();

var er: dynamic = cpp_uninitialized();

var ec: dynamic = cpp_uninitialized();

func dis(r1: dynamic, c1: dynamic, r2: dynamic, c2: dynamic) -> dynamic
{
  return (abs((r1 - r2)) + abs((c1 - c2)));
}

class node
{
  var r: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  var bu: dynamic = cpp_uninitialized();
  var cu: dynamic = cpp_uninitialized();
  var used: dynamic = cpp_uninitialized();
}

var que: dynamic = cpp_uninitialized();

var uu: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(n, m, k);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(mat[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
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
  var now: dynamic = cpp_uninitialized();
  var ne: dynamic = cpp_uninitialized();
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
    var ss: dynamic = now.used;
    var ness: dynamic = cpp_uninitialized();
    var ro: dynamic = now.s;
    var nr: dynamic = now.r;
    var nc: dynamic = now.c;
    uu.clear();
    {
      var i: dynamic = 0;
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
    var ner: dynamic = cpp_uninitialized();
    var nec: dynamic = cpp_uninitialized();
    {
      var i: dynamic = 0;
      while ((i < 4))
      {
        ner = (nr + stran[i][0]);
        nec = (nc + stran[i][1]);
        if (((((ner >= 0) && (ner < n)) && (nec >= 0)) && (nec < m)))
        {
          var p: dynamic = mat[ner][nec];
          ne.r = ner;
          ne.c = nec;
          ne.bu = (now.bu + 1);
          if (((p != cpp_char("T")) && (p != cpp_char("S"))))
          {
            var hu: dynamic = 0;
            {
              var j: dynamic = 0;
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
