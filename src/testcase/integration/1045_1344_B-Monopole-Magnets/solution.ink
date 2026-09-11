// Translated from solution.cpp.

var maxn: dynamic = 1005;

var inf: dynamic = 0x3f3f3f3f;

var m1: dynamic = cpp_array(maxn, maxn);

var m2: dynamic = cpp_array(maxn, maxn);

var hang: dynamic = cpp_array(maxn);

var lie: dynamic = cpp_array(maxn);

var have: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var dxy: dynamic = [[0, 1], [0, -1], [1, 0], [-1, 0]];

var p: dynamic = cpp_uninitialized();

var ch: dynamic = cpp_uninitialized();

func ju1() -> dynamic
{
  var b1: dynamic = 0;
  var b2: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((!hang[i]))
      {
        b1 = 1;
        break;
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      if ((!lie[i]))
      {
        b2 = 1;
        break;
      }
      i += 1;
    }
  }
  if ((b1 && b2))
  {
    return 1;
  }
  if (((!b1) && (!b2)))
  {
    return 1;
  }
  return 0;
}

func ju2(xx: dynamic, yy: dynamic) -> dynamic
{
  var q: dynamic = cpp_uninitialized();
  q.push([xx, yy]);
  var a: dynamic = inf;
  var b: dynamic = (-inf);
  var c: dynamic = inf;
  var d: dynamic = (-inf);
  while (q.size())
  {
    p = q.front();
    q.pop();
    var x: dynamic = p.first;
    var y: dynamic = p.second;
    if (((((((x < 1) || (x > n)) || (y < 1)) || (y > m)) || m1[x][y]) || m2[x][y]))
    {
      continue;
    }
    m2[x][y] = 1;
    m1[x][y] = 1;
    {
      var cpp_name: dynamic = 0;
      while ((cpp_name < 3))
      {
        if ((!m1[(x + dxy[cpp_name][0])][(y + dxy[cpp_name][1])]))
        {
          q.push([(x + dxy[cpp_name][0]), (y + dxy[cpp_name][1])]);
        }
        cpp_name += 1;
      }
    }
  }
  a = 1;
  b = n;
  c = 1;
  d = m;
  {
    var i: dynamic = a;
    while ((i <= b))
    {
      var tmp: dynamic = -1;
      {
        var j: dynamic = c;
        while ((j <= d))
        {
          if (m2[i][j])
          {
            if (((tmp == -1) || (j == (tmp + 1))))
            {
              tmp = j;
            } else
            {
              return 0;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = c;
    while ((i <= d))
    {
      var tmp: dynamic = -1;
      {
        var j: dynamic = a;
        while ((j <= b))
        {
          if (m2[j][i])
          {
            if (((tmp == -1) || (j == (tmp + 1))))
            {
              tmp = j;
            } else
            {
              return 0;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  return 1;
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  read(n, m);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 1;
        while ((j <= m))
        {
          read(ch);
          if ((ch == cpp_char(".")))
          {
            m1[i][j] = 1;
          } else
          {
            have = 1;
            hang[i] = 1;
            lie[j] = 1;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  if ((!ju1()))
  {
    write("-1", "\n");
  } else
  {
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        {
          var j: dynamic = 1;
          while ((j <= m))
          {
            if ((!m1[i][j]))
            {
              if (ju2(i, j))
              {
                ans += 1;
              } else
              {
                ans = -1;
                break;
              }
            }
            j += 1;
          }
        }
        if ((ans == -1))
        {
          break;
        }
        i += 1;
      }
    }
    write(ans, "\n");
  }
  return 0;
}
