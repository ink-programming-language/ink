// Translated from solution.cpp.

var maxn = 1005;

var inf = 0x3f3f3f3f;

var m1 = cpp_array(maxn, maxn);

var m2 = cpp_array(maxn, maxn);

var hang = cpp_array(maxn);

var lie = cpp_array(maxn);

var have: dynamic;

var ans: dynamic;

var n: dynamic;

var m: dynamic;

var dxy = [[0, 1], [0, -1], [1, 0], [-1, 0]];

var p: dynamic;

var ch: dynamic;

func ju1()
{
  var b1 = 0;
  var b2 = 0;
  {
    var i = 1;
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
    var i = 1;
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

func ju2(xx: dynamic, yy: dynamic)
{
  var q: dynamic;
  q.push([xx, yy]);
  var a = inf;
  var b = (-inf);
  var c = inf;
  var d = (-inf);
  while (q.size())
  {
    p = q.front();
    q.pop();
    var x = p.first;
    var y = p.second;
    if (((((((x < 1) || (x > n)) || (y < 1)) || (y > m)) || m1[x][y]) || m2[x][y]))
    {
      continue;
    }
    m2[x][y] = 1;
    m1[x][y] = 1;
    {
      var cpp_name = 0;
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
    var i = a;
    while ((i <= b))
    {
      var tmp = -1;
      {
        var j = c;
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
    var i = c;
    while ((i <= d))
    {
      var tmp = -1;
      {
        var j = a;
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

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  read(n, m);
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 1;
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
      var i = 1;
      while ((i <= n))
      {
        {
          var j = 1;
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
