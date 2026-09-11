// Translated from solution.cpp.

var N: dynamic = (1e5 + 10);

var alpha: dynamic = 128;

var n: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var x: dynamic = cpp_uninitialized();

var y: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N);

var mx: dynamic = -1;

var mn: dynamic = 1e9;

var sum: dynamic = cpp_array(N);

var s: dynamic = cpp_uninitialized();

var s1: dynamic = cpp_uninitialized();

var s2: dynamic = cpp_uninitialized();

var trie: dynamic = cpp_array(alpha, N);

var ndcnt: dynamic = cpp_uninitialized();

var ids: dynamic = cpp_array(N);

var fail: dynamic = cpp_array(N);

var sz: dynamic = cpp_array(N);

var nxtid: dynamic = cpp_array(N);

var myids: dynamic = cpp_array(N);

var children: dynamic = cpp_array(alpha, N);

func addnode() -> dynamic
{
  memset(trie[ndcnt], -1, cpp_sizeof((trie[ndcnt])));
  ids[ndcnt] = -1;
  sz[ndcnt] = 0;
  ndcnt += 1;
  return;
}

func insert(str: dynamic, id: dynamic) -> dynamic
{
  var cur: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
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

func getnxt(f: dynamic, c: dynamic) -> dynamic
{
  while ((trie[f][c] == -1))
  {
    f = fail[f];
  }
  f = trie[f][c];
  return f;
}

func buildfail() -> dynamic
{
  var qu: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < alpha))
    {
      var r: dynamic = trie[0][i];
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
    var cur: dynamic = qu.front();
    qu.pop();
    {
      var i: dynamic = 0;
      while ((i < sz[cur]))
      {
        var c: dynamic = children[cur][i];
        nxtid[trie[cur][c]] = cpp_assign(fail[trie[cur][c]], "=", getnxt(fail[cur], c));
        qu.push(trie[cur][c]);
        i += 1;
      }
    }
  }
}

func getnxtid(cur: dynamic) -> dynamic
{
  if ((cur == 0))
  {
    return 0;
  }
  var nxt: dynamic = nxtid[cur];
  if ((ids[nxt] != -1))
  {
    return nxt;
  }
  return cpp_assign(nxt, "=", getnxtid(nxt));
}

func init() -> dynamic
{
  ndcnt = 0;
  addnode();
}

var SZ: dynamic = 205;

class matrix
{
  var a: dynamic = cpp_array(SZ, SZ);
  func matrix() -> dynamic
  {
      {
        var i: dynamic = 0;
        while ((i < SZ))
        {
          {
            var j: dynamic = 0;
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
  func operator_multiply(other: dynamic) -> dynamic
  {
      var ret: dynamic = cpp_uninitialized();
      {
        var i: dynamic = 0;
        while ((i < SZ))
        {
          {
            var j: dynamic = 0;
            while ((j < SZ))
            {
              {
                var k: dynamic = 0;
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
  func POW(b: dynamic) -> dynamic
  {
      var res: dynamic = cpp_uninitialized();
      var a: dynamic = ((*self));
      {
        var i: dynamic = 0;
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

func main() -> dynamic
{
  init();
  read(n, k);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(s1);
      x = i;
      myids[i] = insert(s1, i);
      i += 1;
    }
  }
  buildfail();
  var ans: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < ndcnt))
    {
      {
        var c: dynamic = cpp_char("a");
        while ((c <= cpp_char("z")))
        {
          var nxt: dynamic = getnxt(i, c);
          var cnt: dynamic = 0;
          {
            var j: dynamic = nxt;
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
    var i: dynamic = 0;
    while ((i < SZ))
    {
      mx = max(mx, ans.a[i][0]);
      i += 1;
    }
  }
  write(mx, "\n");
}
