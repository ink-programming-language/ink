// Translated from solution.cpp.

var N = (4e5 + 10);

var M = (3e8 + 1);

var big = 1e18;

var shift = 1000;

var hsh2 = 1964325029;

var mod = 998244353;

var EPS = 1e-14;

var block = 700;

var n: dynamic;

var m: dynamic;

var Z = cpp_array(N);

var q = cpp_array(N);

class aho
{
  var t: dynamic = cpp_array(26, N);
  var b: dynamic = cpp_array(N);
  var p: dynamic = cpp_array(N);
  var tp: dynamic = cpp_array(N);
  var k: dynamic = cpp_array(N);
  var cur: dynamic;
  var songs: dynamic = cpp_array(N);
  var term: dynamic = cpp_array(N);
  func add1(a: dynamic, num: dynamic)
  {
      var v = 1;
      for (var u in a)
      {
        var ch = (u - cpp_char("a"));
        if ((!t[v][ch]))
        {
          b[cur] = ch;
          p[cur] = v;
          t[v][ch] = cpp_update(cur, "++");
        }
        v = t[v][ch];
      }
      songs[num] = v;
      term[v] = 1;
    }
  func add2(j: dynamic, z: dynamic, num: dynamic)
  {
      j = songs[j];
      if ((!t[j][z]))
      {
        b[cur] = z;
        p[cur] = j;
        t[j][z] = cpp_update(cur, "++");
      }
      songs[num] = t[j][z];
    }
  func suflink(v: dynamic)
  {
      if ((p[v] != 1))
      {
        k[v] = t[k[p[v]]][b[v]];
      } else
      {
        k[v] = 1;
      }
      if (term[k[v]])
      {
        tp[v] = k[v];
      } else
      {
        tp[v] = tp[k[v]];
      }
      {
        var i = 0;
        while ((i < 26))
        {
          if (t[v][i])
          {
            i += 1;
            continue;
          }
          t[v][i] = t[k[v]][i];
          i += 1;
        }
      }
    }
  func sufbld()
  {
      p[1] = 1;
      k[1] = 1;
      tp[1] = 1;
      {
        var i = 0;
        while ((i < 26))
        {
          if ((!t[1][i]))
          {
            t[1][i] = 1;
          }
          i += 1;
        }
      }
      var q: dynamic;
      q.push(1);
      while (q.size())
      {
        var v = q.front();
        q.pop();
        {
          var i = 0;
          while ((i < 26))
          {
            if ((t[v][i] && (t[v][i] != 1)))
            {
              q.push(t[v][i]);
            }
            i += 1;
          }
        }
        if ((v != 1))
        {
          suflink(v);
        }
      }
    }
  func upd(z: dynamic, d: dynamic)
  {
      while ((z != 1))
      {
        Z[z] += d;
        z = tp[z];
      }
    }
}

var rt = cpp_array(2);

var answ = cpp_array(N);

func dfs(a: dynamic, b: dynamic)
{
  rt[1].upd(b, 1);
  for (var u in q[a])
  {
    answ[u.first] = Z[u.second];
  }
  {
    var i = 0;
    while ((i < 26))
    {
      if (rt[0].t[a][i])
      {
        dfs(rt[0].t[a][i], rt[1].t[b][i]);
      }
      i += 1;
    }
  }
  rt[1].upd(b, -1);
}

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  read(n);
  {
    var i = 1;
    var j: dynamic;
    var z: dynamic;
    while ((i <= n))
    {
      var x: dynamic;
      read(j);
      if ((j == 1))
      {
        j = 0;
        read(x);
      } else
      {
        read(j, x);
      }
      rt[0].add2(j, (x - cpp_char("a")), i);
      i += 1;
    }
  }
  read(m);
  var x: dynamic;
  {
    var i = 1;
    var y: dynamic;
    while ((i <= m))
    {
      read(y, x);
      rt[1].add1(x, i);
      q[rt[0].songs[y]].push_back(make_pair(i, rt[1].songs[i]));
      i += 1;
    }
  }
  rt[1].sufbld();
  dfs(1, 1);
  {
    var i = 1;
    while ((i <= m))
    {
      write(answ[i], cpp_char("\n"));
      i += 1;
    }
  }
}
