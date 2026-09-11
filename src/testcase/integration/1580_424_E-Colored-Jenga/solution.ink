// Translated from solution.cpp.

func operator_shift_left(s: dynamic, p: dynamic) -> dynamic
{
  return (((((s << "(") << p.first) << ",") << p.second) << ")");
}

func operator_shift_left(s: dynamic, c: dynamic) -> dynamic
{
  (s << "[ ");
  for (var it: dynamic in c)
  {
    ((s << it) << " ");
  }
  (s << "]");
  return s;
}

var N: dynamic = cpp_uninitialized();

var second: dynamic = cpp_uninitialized();

var MAXS: dynamic = 1510101;

var INF: dynamic = 1E20;

var prob: dynamic = [(1.0 / 6.0), (1.0 / 3.0), (1.0 / 3.0)];

var arr: dynamic = cpp_array(4, 6);

var states: dynamic = cpp_array(MAXS);

var mp: dynamic = cpp_uninitialized();

var dp: dynamic = cpp_array(MAXS);

var isterm: dynamic = cpp_array(MAXS);

var trans: dynamic = cpp_array(3, MAXS);

var ladd: dynamic = cpp_array(3, 64);

var ldel: dynamic = cpp_array(3, 64);

var lok: dynamic = cpp_array(64);

var lfull: dynamic = cpp_array(64);

var zv: dynamic = cpp_array(64);

var yv: dynamic = cpp_array(64);

func lrand() -> dynamic
{
  return (((rand() * cpp_cast(RAND_MAX))) + rand());
}

func llrand() -> dynamic
{
  return (((lrand() * cpp_cast(RAND_MAX)) * RAND_MAX) + lrand());
}

func lmin(z: dynamic) -> dynamic
{
  var n: dynamic = ((((z & 0xc)) | ((((z & 0x3)) << 4))) | ((((z & 0x30)) >> 4)));
  return min(n, z);
}

func makel() -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < 64))
    {
      lok[i] = cpp_binary(cpp_binary(((i & 0xf)), "and", ((i & 0x3c))), "and", (i == lmin(i)));
      lfull[i] = cpp_binary(cpp_binary(((i & 0x3)), "and", ((i & 0xc))), "and", ((i & 0x30)));
      zv[i] = (llrand() % cpp_cast(1e16));
      yv[i] = (llrand() % cpp_cast(1e16));
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < 64))
    {
      {
        var p: dynamic = 0;
        while ((p < 3))
        {
          if ((!lok[i]))
          {
            p += 1;
            continue;
          }
          var clr: dynamic = (((i >> ((2 * p)))) & 3);
          if ((!clr))
          {
            p += 1;
            continue;
          }
          var ns: dynamic = lmin((i - ((clr << ((2 * p))))));
          if (lok[ns])
          {
            ldel[i][(clr - 1)].push_back(ns);
          }
          p += 1;
        }
      }
      {
        var p: dynamic = 0;
        while ((p < 3))
        {
          if ((((i >> ((2 * p)))) & 3))
          {
            p += 1;
            continue;
          }
          {
            var c: dynamic = 0;
            while ((c < 3))
            {
              var ns: dynamic = lmin((i + ((((c + 1)) << ((2 * p))))));
              ladd[i][c].push_back(ns);
              c += 1;
            }
          }
          p += 1;
        }
      }
      {
        var c: dynamic = 0;
        while ((c < 3))
        {
          sort(begin(ladd[i][c]), end(ladd[i][c]));
          sort(begin(ldel[i][c]), end(ldel[i][c]));
          ladd[i][c].resize((unique(begin(ladd[i][c]), end(ladd[i][c])) - ladd[i][c].begin()));
          ldel[i][c].resize((unique(begin(ldel[i][c]), end(ldel[i][c])) - ldel[i][c].begin()));
          c += 1;
        }
      }
      i += 1;
    }
  }
}

func decode(x: dynamic) -> dynamic
{
  var ret: dynamic = cpp_uninitialized();
  while (x)
  {
    ret.push_back((x & 0x3f));
    x >>= 6;
  }
  return ret;
}

func encode(v: dynamic) -> dynamic
{
  var res: dynamic = 0;
  var sz: dynamic = (cpp_cast((v).size()));
  {
    var i: dynamic = (sz - 1);
    while ((i >= 0))
    {
      res = (((res << 6)) + v[i]);
      i -= 1;
    }
  }
  return res;
}

func enhash(v: dynamic) -> dynamic
{
  var sz: dynamic = (cpp_cast((v).size()));
  var res: dynamic = yv[v[(sz - 1)]];
  {
    var i: dynamic = 0;
    while ((i < (sz - 1)))
    {
      if (cpp_binary(((v[i] & 0xc)), "and", (cpp_binary(((v[i] & 0x30)), "or", ((v[i] & 0x3))))))
      {
        res += zv[v[i]];
      }
      i += 1;
    }
  }
  return res;
}

func normalize(v: dynamic) -> dynamic
{
  var sz: dynamic = (cpp_cast((v).size()));
  var nv: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < (sz - 1)))
    {
      if (cpp_binary(((v[i] & 0xc)), "and", (cpp_binary(((v[i] & 0x30)), "or", ((v[i] & 0x3))))))
      {
        nv.push_back(v[i]);
      }
      i += 1;
    }
  }
  sort(begin(nv), end(nv));
  nv.push_back(v[(sz - 1)]);
  return nv;
}

func go(s: dynamic) -> dynamic
{
  if ((dp[s] != INF))
  {
    return dp[s];
  }
  if (cpp_binary(cpp_binary(trans[s][0].empty(), "and", trans[s][1].empty()), "and", trans[s][2].empty()))
  {
    dp[s] = 0;
  } else
  {
    var best: dynamic = [INF, INF, INF];
    {
      var j: dynamic = 0;
      while ((j < 3))
      {
        for (var ns: dynamic in trans[s][j])
        {
          best[j] = min(best[j], go(ns));
        }
        j += 1;
      }
    }
    var okprob: dynamic = 0;
    var pg: dynamic = 0;
    {
      var j: dynamic = 0;
      while ((j < 3))
      {
        if ((best[j] != INF))
        {
          okprob += prob[j];
          pg += (prob[j] * best[j]);
        }
        j += 1;
      }
    }
    dp[s] = (((1.0 + pg)) / okprob);
  }
  return dp[s];
}

func s2s(v: dynamic) -> dynamic
{
  var ret: dynamic = "\n";
  var sz: dynamic = (cpp_cast((v).size()));
  {
    var j: dynamic = (sz - 1);
    while ((j >= 0))
    {
      var x: dynamic = v[j];
      {
        var i: dynamic = 0;
        while ((i < 3))
        {
          var y: dynamic = (x & 3);
          ret.push_back( ((y == 1)) ? cpp_char("R") : ( ((y == 2)) ? cpp_char("G") : ( ((y == 0)) ? cpp_char(" ") : cpp_char("B"))));
          x >>= 2;
          i += 1;
        }
      }
      ret += "\n";
      j -= 1;
    }
  }
  ret.pop_back();
  return ret;
}

func calc() -> dynamic
{
  makel();
  var sv: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      var a: dynamic = cpp_array(3);
      {
        var j: dynamic = 0;
        while ((j < 3))
        {
          a[j] = ( ((arr[i][j] == cpp_char("R"))) ? 1 : ( ((arr[i][j] == cpp_char("G"))) ? 2 : 3));
          j += 1;
        }
      }
      var z: dynamic = (((a[0] + ((a[1] << 2))) + ((a[2] << 4))));
      sv.push_back(lmin(z));
      i += 1;
    }
  }
  var hs: dynamic = enhash(sv);
  sv = normalize(sv);
  var hv: dynamic = encode(sv);
  second = 0;
  states[second] = hv;
  mp[hs] = second;
  second += 1;
  {
    var pos: dynamic = 0;
    while ((pos < second))
    {
      var h: dynamic = states[pos];
      var v: dynamic = decode(h);
      var sz: dynamic = (cpp_cast((v).size()));
      if ((sz == 1))
      {
        pos += 1;
        continue;
      }
      var nxt: dynamic = (sz - 1);
      if (lfull[v[(sz - 1)]])
      {
        v.push_back(0);
        nxt += 1;
      }
      {
        var i: dynamic = 0;
        while ((i < (sz - 1)))
        {
          {
            var c: dynamic = 0;
            while ((c < 3))
            {
              for (var x: dynamic in ldel[v[i]][c])
              {
                var ov: dynamic = v[i];
                v[i] = x;
                for (var y: dynamic in ladd[v[nxt]][c])
                {
                  var on: dynamic = v[nxt];
                  v[nxt] = y;
                  var hh: dynamic = enhash(v);
                  if ((!mp.count(hh)))
                  {
                    var ns: dynamic = normalize(v);
                    var nh: dynamic = encode(ns);
                    states[second] = nh;
                    mp[hh] = second;
                    second += 1;
                  }
                  var nid: dynamic = mp[hh];
                  trans[pos][c].push_back(nid);
                  v[nxt] = on;
                }
                v[i] = ov;
              }
              c += 1;
            }
          }
          i += 1;
        }
      }
      {
        var c: dynamic = 0;
        while ((c < 3))
        {
          trans[pos][c].resize((unique(begin(trans[pos][c]), end(trans[pos][c])) - trans[pos][c].begin()));
          c += 1;
        }
      }
      pos += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < second))
    {
      isterm[i] = (cpp_binary(cpp_binary(trans[i][0].empty(), "and", trans[i][1].empty()), "and", trans[i][2].empty()));
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < second))
    {
      dp[i] = INF;
      i += 1;
    }
  }
  var ans: dynamic = go(0);
  return ans;
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  read(N);
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      read(arr[i]);
      i += 1;
    }
  }
  var ans: dynamic = calc();
  write(fixed, setprecision(10), ans, "\n");
  return 0;
}
