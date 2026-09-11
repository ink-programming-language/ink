// Translated from solution.cpp.

var h: dynamic = cpp_uninitialized();

var w: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var grid: dynamic = cpp_uninitialized();

class Data
{
  var p: dynamic = cpp_uninitialized();
  var bs: dynamic = cpp_uninitialized();
  func Data(p0: dynamic, bs0: dynamic) -> dynamic
  {
      p = p0;
      bs = bs0;
    }
  func toInt() -> dynamic
  {
      var ret: dynamic = bs.to_ulong();
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          ret *= (h * w);
          ret += p[i];
          i += 1;
        }
      }
      return ret;
    }
}

func solve() -> dynamic
{
  var gridLine: dynamic = accumulate(grid.begin(), grid.end(), string_cpp());
  var diff: dynamic = [1, -1, w, (-w)];
  {
    var i: dynamic = 0;
    while ((i < h))
    {
      {
        var j: dynamic = 0;
        while ((j < w))
        {
          var c: dynamic = grid[i][j];
          if (((cpp_char("a") <= c) && (c <= cpp_char("c"))))
          {
            sp[(c - cpp_char("a"))] = ((i * w) + j);
          } else if (((cpp_char("A") <= c) && (c <= cpp_char("C"))))
          {
            gp[(c - cpp_char("A"))] = ((i * w) + j);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  var size: dynamic = (1 << n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      size *= (h * w);
      i += 1;
    }
  }
  var check: dynamic = cpp_construct(2, vector(size, false));
  check[0][Data(sp, 0).toInt()] = true;
  check[1][Data(gp, 0).toInt()] = true;
  var dq: dynamic = cpp_construct(2);
  dq[0].push_back(Data(sp, 0));
  dq[1].push_back(Data(gp, 0));
  var turn: dynamic = 0;
  var m: dynamic = 1;
  var ret: dynamic = 1;
  {
    while (true)
    {
      if ((m == 0))
      {
        ret += 1;
        turn ^= 1;
        m = dq[turn].size();
      }
      var d: dynamic = dq[turn].front();
      dq[turn].pop_front();
      m -= 1;
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          if (d.bs[i])
          {
            i += 1;
            continue;
          }
          d.bs[i] = true;
          {
            var j: dynamic = 0;
            while ((j < 4))
            {
              d.p[i] += diff[j];
              var ok: dynamic = true;
              if ((gridLine[d.p[i]] == cpp_char("#")))
              {
                ok = false;
              }
              {
                var k: dynamic = 0;
                while ((k < n))
                {
                  if (((k != i) && (d.p[k] == d.p[i])))
                  {
                    ok = false;
                  }
                  k += 1;
                }
              }
              if (ok)
              {
                var a: dynamic = d.toInt();
                if ((!check[turn][a]))
                {
                  dq[turn].push_front(d);
                  m += 1;
                  check[turn][a] = true;
                }
              }
              d.p[i] -= diff[j];
              j += 1;
            }
          }
          d.bs[i] = false;
          i += 1;
        }
      }
      d.bs = 0;
      var a: dynamic = d.toInt();
      if ((!check[turn][a]))
      {
        if (check[(turn ^ 1)][a])
        {
          return ret;
        }
        dq[turn].push_back(d);
        check[turn][a] = true;
      }
    }
  }
}

func main() -> dynamic
{
  {
    while (true)
    {
      read(w, h, n);
      if ((w == 0))
      {
        return 0;
      }
      cin.ignore();
      grid.resize(h);
      {
        var i: dynamic = 0;
        while ((i < h))
        {
          getline(cin, grid[i]);
          i += 1;
        }
      }
      write(solve(), "\n");
    }
  }
}
