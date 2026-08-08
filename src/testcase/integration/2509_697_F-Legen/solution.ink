// Translated from solution.cpp.

var N = (1e5 + 10);

var alpha = 128;

var n: dynamic;

var q: dynamic;

var m: dynamic;

var k: dynamic;

var x: dynamic;

var y: dynamic;

var a = cpp_array(N);

var mx = -1;

var mn = 1e9;

var sum = cpp_array(N);

var s: dynamic;

var s1: dynamic;

var s2: dynamic;

var trie = cpp_array(alpha, N);

var ndcnt: dynamic;

var ids = cpp_array(N);

var fail = cpp_array(N);

var sz = cpp_array(N);

var nxtid = cpp_array(N);

var myids = cpp_array(N);

var children = cpp_array(alpha, N);

func addnode()
{
  memset(trie[ndcnt], -1, cpp_sizeof((trie[ndcnt])));
  ids[ndcnt] = -1;
  sz[ndcnt] = 0;
  ndcnt += 1;
  return;
}

func insert(str: dynamic, id: dynamic)
{
  var cur: dynamic;
  var i: dynamic;
  {
    cur = 0;
    i = 0;
    while ((i < str.size()))
    {
      if ((trie[cur][str[i]] == -1))
      {
        trie[cur][str[i]] = ndcnt;
        addnode();
        children[cur][cpp_update(sz[cur], "++")] = str[i];
      }
      cur = trie[cur][str[i]];
      i += 1;
    }
  }
  if ((ids[cur] == -1))
  {
    ids[cur] = id;
  }
  sum[ids[cur]] += a[x];
  return ids[cur];
}

func getnxt(f: dynamic, c: dynamic)
{
  while ((trie[f][c] == -1))
  {
    f = fail[f];
  }
  f = trie[f][c];
  return f;
}

func buildfail()
{
  var qu: dynamic;
  {
    var i = 0;
    while ((i < alpha))
    {
      var r = trie[0][i];
      if ((r != -1))
      {
        fail[r] = 0;
        nxtid[r] = 0;
        qu.push(r);
      } else
      {
        trie[0][i] = 0;
      }
      i += 1;
    }
  }
  while (qu.size())
  {
    var cur = qu.front();
    qu.pop();
    {
      var i = 0;
      while ((i < sz[cur]))
      {
        var c = children[cur][i];
        nxtid[trie[cur][c]] = cpp_assign(fail[trie[cur][c]], "=", getnxt(fail[cur], c));
        qu.push(trie[cur][c]);
        i += 1;
      }
    }
  }
}

func getnxtid(cur: dynamic)
{
  if ((cur == 0))
  {
    return 0;
  }
  var nxt = nxtid[cur];
  if ((ids[nxt] != -1))
  {
    return nxt;
  }
  return cpp_assign(nxt, "=", getnxtid(nxt));
}

func init()
{
  ndcnt = 0;
  addnode();
}

var SZ = 205;

class matrix
{
  var a: dynamic = cpp_array(SZ, SZ);
  func matrix()
  {
      {
        var i = 0;
        while ((i < SZ))
        {
          {
            var j = 0;
            while ((j < SZ))
            {
              a[i][j] = -1e9;
              j += 1;
            }
          }
          i += 1;
        }
      }
    }
  func operator_multiply(other: dynamic)
  {
      var ret: dynamic;
      {
        var i = 0;
        while ((i < SZ))
        {
          {
            var j = 0;
            while ((j < SZ))
            {
              {
                var k = 0;
                while ((k < SZ))
                {
                  ret.a[i][k] = max(ret.a[i][k], (a[i][j] + other.a[j][k]));
                  k += 1;
                }
              }
              j += 1;
            }
          }
          i += 1;
        }
      }
      return ret;
    }
  func POW(b: dynamic)
  {
      var res: dynamic;
      var a = ((*this));
      {
        var i = 0;
        while ((i < SZ))
        {
          res.a[i][i] = 0;
          i += 1;
        }
      }
      while (b)
      {
        if ((b & 1))
        {
          res = (res * a);
        }
        a = (a * a);
        b /= 2;
      }
      return res;
    }
}

func main()
{
  init();
  read(n, k);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      read(s1);
      x = i;
      myids[i] = insert(s1, i);
      i += 1;
    }
  }
  buildfail();
  var ans: dynamic;
  {
    var i = 0;
    while ((i < ndcnt))
    {
      {
        var c = cpp_char("a");
        while ((c <= cpp_char("z")))
        {
          var nxt = getnxt(i, c);
          var cnt = 0;
          {
            var j = nxt;
            while (j)
            {
              if ((ids[j] != -1))
              {
                cnt += sum[ids[j]];
              }
              j = getnxtid(j);
            }
          }
          ans.a[nxt][i] = max(ans.a[nxt][i], cnt);
          c += 1;
        }
      }
      i += 1;
    }
  }
  ans = ans.POW(k);
  {
    var i = 0;
    while ((i < SZ))
    {
      mx = max(mx, ans.a[i][0]);
      i += 1;
    }
  }
  write(mx, "\n");
}
