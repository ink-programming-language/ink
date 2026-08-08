// Translated from solution.cpp.

var h: dynamic;

var w: dynamic;

var n: dynamic;

var grid: dynamic;

class Data
{
  var p: dynamic;
  var bs: dynamic;
  func Data(p0: dynamic, bs0: dynamic)
  {
      p = p0;
      bs = bs0;
    }
  func toInt()
  {
      var ret = bs.to_ulong();
      {
        var i = 0;
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

func solve()
{
  var gridLine = accumulate(grid.begin(), grid.end(), string_cpp());
  var diff = [1, -1, w, (-w)];
  {
    var i = 0;
    while ((i < h))
    {
      {
        var j = 0;
        while ((j < w))
        {
          var c = grid[i][j];
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
  var size = (1 << n);
  {
    var i = 0;
    while ((i < n))
    {
      size *= (h * w);
      i += 1;
    }
  }
  var check = cpp_construct(2, vector(size, false));
  check[0][Data(sp, 0).toInt()] = true;
  check[1][Data(gp, 0).toInt()] = true;
  var dq = cpp_construct(2);
  dq[0].push_back(Data(sp, 0));
  dq[1].push_back(Data(gp, 0));
  var turn = 0;
  var m = 1;
  var ret = 1;
  {
    while (true)
    {
      if ((m == 0))
      {
        ret += 1;
        turn ^= 1;
        m = dq[turn].size();
      }
      var d = dq[turn].front();
      dq[turn].pop_front();
      m -= 1;
      {
        var i = 0;
        while ((i < n))
        {
          if (d.bs[i])
          {
            i += 1;
            continue;
          }
          d.bs[i] = true;
          {
            var j = 0;
            while ((j < 4))
            {
              d.p[i] += diff[j];
              var ok = true;
              if ((gridLine[d.p[i]] == cpp_char("#")))
              {
                ok = false;
              }
              {
                var k = 0;
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
                var a = d.toInt();
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
      var a = d.toInt();
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

func main()
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
        var i = 0;
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
