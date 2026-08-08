// Translated from solution.cpp.

func operator_shift_right(is: dynamic, v: dynamic)
{
  for (var x in v)
  {
    (is >> x);
  }
  return is;
}

func operator_shift_left(os: dynamic, v: dynamic)
{
  if ((!v.empty()))
  {
    (os << v.front());
    {
      var x = 1;
      while ((x < v.size()))
      {
        ((os << cpp_char(" ")) << v[x]);
        x += 1;
      }
    }
  }
  return os;
}

func gg()
{
  write("-1\n");
  exit(0);
}

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  var n: dynamic;
  var m: dynamic;
  var k: dynamic;
  read(n, m, k);
  var ans = cpp_construct(n, vector(m, vector(k, -1)));
  var pxn = cpp_construct(m, vector(k, (n - 1)));
  var pyn = cpp_construct(n, vector(k, (m - 1)));
  var pzn = cpp_construct(n, vector(m, (k - 1)));
  read(xp, xn, yp, yn, zp, zn);
  var q: dynamic;
  {
    var x = 0;
    while ((x < n))
    {
      {
        var y = 0;
        while ((y < m))
        {
          {
            var z = 0;
            while ((z < k))
            {
              if ((!((((((xp[y][z] && xn[y][z]) && yp[x][z]) && yn[x][z]) && zp[x][y]) && zn[x][y]))))
              {
                ans[x][y][z] = 0;
              }
              z += 1;
            }
          }
          y += 1;
        }
      }
      x += 1;
    }
  }
  var exx = __cpp_lambda_1;
  var exy = __cpp_lambda_2;
  var exz = __cpp_lambda_3;
  {
    var y = 0;
    while ((y < m))
    {
      {
        var z = 0;
        while ((z < k))
        {
          if ((bool_cpp(xp[y][z]) != bool_cpp(xn[y][z])))
          {
            gg();
          }
          if ((!xp[y][z]))
          {
            z += 1;
            continue;
          }
          exx(y, z);
          z += 1;
        }
      }
      y += 1;
    }
  }
  {
    var x = 0;
    while ((x < n))
    {
      {
        var z = 0;
        while ((z < k))
        {
          if ((bool_cpp(yp[x][z]) != bool_cpp(yn[x][z])))
          {
            gg();
          }
          if ((!yp[x][z]))
          {
            z += 1;
            continue;
          }
          exy(x, z);
          z += 1;
        }
      }
      x += 1;
    }
  }
  {
    var x = 0;
    while ((x < n))
    {
      {
        var y = 0;
        while ((y < m))
        {
          if ((bool_cpp(zp[x][y]) != bool_cpp(zn[x][y])))
          {
            gg();
          }
          if ((!zp[x][y]))
          {
            y += 1;
            continue;
          }
          exz(x, y);
          y += 1;
        }
      }
      x += 1;
    }
  }
  while ((!q.empty()))
  {
    var x: dynamic;
    var y: dynamic;
    var z: dynamic;
    tie(x, y, z) = q.front();
    q.pop();
    if ((ans[x][y][z] == 0))
    {
      continue;
    }
    var col: dynamic;
    if ((pxp[y][z] == x))
    {
      col.push_back(xp[y][z]);
    }
    if ((pxn[y][z] == x))
    {
      col.push_back(xn[y][z]);
    }
    if ((pyp[x][z] == y))
    {
      col.push_back(yp[x][z]);
    }
    if ((pyn[x][z] == y))
    {
      col.push_back(yn[x][z]);
    }
    if ((pzp[x][y] == z))
    {
      col.push_back(zp[x][y]);
    }
    if ((pzn[x][y] == z))
    {
      col.push_back(zn[x][y]);
    }
    if ((col.size() == 0))
    {
      continue;
    }
    col.erase(unique(col.begin(), col.end()), col.end());
    if ((col.size() == 1))
    {
      ans[x][y][z] = col.back();
      continue;
    }
    ans[x][y][z] = 0;
    exx(y, z);
    exy(x, z);
    exz(x, y);
  }
  {
    var x = 0;
    while ((x < n))
    {
      {
        var y = 0;
        while ((y < m))
        {
          {
            var z = 0;
            while ((z < k))
            {
              if ((ans[x][y][z] == -1))
              {
                ans[x][y][z] = 0;
              }
              z += 1;
            }
          }
          write(ans[x][y], cpp_char("\n"));
          y += 1;
        }
      }
      write(cpp_char("\n"));
      x += 1;
    }
  }
  return 0;
}

func __cpp_lambda_1(y: dynamic, z: dynamic)
{
  while (((pxp[y][z] < n) && (!ans[pxp[y][z]][y][z])))
  {
    pxp[y][z] += 1;
  }
  while (((pxn[y][z] >= 0) && (!ans[pxn[y][z]][y][z])))
  {
    pxn[y][z] -= 1;
  }
  if (((pxp[y][z] >= n) || (pxn[y][z] < 0)))
  {
    gg();
  }
  q.emplace(pxp[y][z], y, z);
  q.emplace(pxn[y][z], y, z);
}

func __cpp_lambda_2(x: dynamic, z: dynamic)
{
  while (((pyp[x][z] < m) && (!ans[x][pyp[x][z]][z])))
  {
    pyp[x][z] += 1;
  }
  while (((pyn[x][z] >= 0) && (!ans[x][pyn[x][z]][z])))
  {
    pyn[x][z] -= 1;
  }
  if (((pyp[x][z] >= m) || (pyn[x][z] < 0)))
  {
    gg();
  }
  q.emplace(x, pyp[x][z], z);
  q.emplace(x, pyn[x][z], z);
}

func __cpp_lambda_3(x: dynamic, y: dynamic)
{
  while (((pzp[x][y] < k) && (!ans[x][y][pzp[x][y]])))
  {
    pzp[x][y] += 1;
  }
  while (((pzn[x][y] >= 0) && (!ans[x][y][pzn[x][y]])))
  {
    pzn[x][y] -= 1;
  }
  if (((pzp[x][y] >= k) || (pzn[x][y] < 0)))
  {
    gg();
  }
  q.emplace(x, y, pzp[x][y]);
  q.emplace(x, y, pzn[x][y]);
}
