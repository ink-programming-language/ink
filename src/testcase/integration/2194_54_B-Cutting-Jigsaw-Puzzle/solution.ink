// Translated from solution.cpp.

func max(i: dynamic, j: dynamic)
{
  return if ((i > j)) i else j;
}

var v1: dynamic;

var v2: dynamic;

var c = cpp_array(25, 25);

var minx: dynamic;

var miny: dynamic;

var a: dynamic;

var b: dynamic;

var ans: dynamic;

var f: dynamic;

func ju(x1: dynamic, y1: dynamic, x2: dynamic, y2: dynamic, x: dynamic, y: dynamic)
{
  var cp1 = cpp_array(25, 25);
  var cp2 = cpp_array(25, 25);
  var i: dynamic;
  var j: dynamic;
  var flag1 = 0;
  var flag2 = 0;
  var flag3 = 0;
  var flag4 = 0;
  {
    i = 0;
    while ((i < x))
    {
      {
        j = 0;
        while ((j < y))
        {
          cp1[i][j] = c[(x1 + i)][(y1 + j)];
          cp2[i][j] = c[(x2 + i)][(y2 + j)];
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    i = 0;
    while ((i < x))
    {
      {
        j = 0;
        while ((j < y))
        {
          if ((c[(x1 + i)][(y1 + j)] != c[(x2 + i)][(y2 + j)]))
          {
            flag1 += 1;
            break;
          }
          j += 1;
        }
      }
      if ((flag1 == 1))
      {
        break;
      }
      i += 1;
    }
  }
  {
    i = 0;
    while ((i < x))
    {
      {
        j = 0;
        while ((j < y))
        {
          if ((c[(x1 + i)][(y1 + j)] != c[(((x2 + x) - i) - 1)][(((y2 + y) - j) - 1)]))
          {
            flag2 += 1;
            break;
          }
          j += 1;
        }
      }
      if ((flag2 == 1))
      {
        break;
      }
      i += 1;
    }
  }
  if ((x == y))
  {
    if ((x == 1))
    {
      flag3 = 1;
      flag4 = 1;
    } else
    {
      {
        i = 0;
        while ((i < x))
        {
          {
            j = 0;
            while ((j < y))
            {
              if ((cp1[i][j] != cp2[j][((x - i) - 1)]))
              {
                flag3 += 1;
                break;
              }
              j += 1;
            }
          }
          if ((flag3 == 1))
          {
            break;
          }
          i += 1;
        }
      }
      {
        i = 0;
        while ((i < x))
        {
          {
            j = 0;
            while ((j < y))
            {
              if ((cp1[i][j] != cp2[((y - j) - 1)][i]))
              {
                flag4 += 1;
                break;
              }
              j += 1;
            }
          }
          if ((flag4 == 1))
          {
            break;
          }
          i += 1;
        }
      }
    }
  }
  if ((x == y))
  {
    if ((((flag1 && flag2) && flag3) && flag4))
    {
      return 1;
    } else
    {
      return 0;
    }
  } else
  {
    if ((flag1 && flag2))
    {
      return 1;
    } else
    {
      return 0;
    }
  }
}

func cnt(x: dynamic, y: dynamic)
{
  var xi: dynamic;
  var xj: dynamic;
  var yi: dynamic;
  var yj: dynamic;
  {
    xi = 0;
    while ((xi < a))
    {
      {
        yi = 0;
        while ((yi < b))
        {
          {
            xj = 0;
            while ((xj < a))
            {
              {
                yj = 0;
                while ((yj < b))
                {
                  if (((xi == xj) && (yi == yj)))
                  {
                    yj += y;
                    continue;
                  }
                  if ((ju(xi, yi, xj, yj, x, y) == 0))
                  {
                    f = 1;
                    break;
                  }
                  yj += y;
                }
              }
              if (f)
              {
                break;
              }
              xj += x;
            }
          }
          if (f)
          {
            break;
          }
          yi += y;
        }
      }
      if (f)
      {
        break;
      }
      xi += x;
    }
  }
  if ((!f))
  {
    ans += 1;
  }
}

func main()
{
  var i: dynamic;
  var j: dynamic;
  while ((scanf("%d%d", (&a), (&b)) != EOF))
  {
    ans = 0;
    minx = a;
    miny = b;
    {
      i = 0;
      while ((i < a))
      {
        scanf(" %s", c[i]);
        i += 1;
      }
    }
    v1.clear();
    v2.clear();
    {
      i = 1;
      while ((i <= a))
      {
        if (((a % i) == 0))
        {
          v1.push_back(i);
        }
        i += 1;
      }
    }
    {
      i = 1;
      while ((i <= b))
      {
        if (((b % i) == 0))
        {
          v2.push_back(i);
        }
        i += 1;
      }
    }
    {
      i = 0;
      while ((i < v1.size()))
      {
        {
          j = 0;
          while ((j < v2.size()))
          {
            f = 0;
            cnt(v1[i], v2[j]);
            if ((!f))
            {
              if (((v1[i] * v2[j]) < (minx * miny)))
              {
                minx = v1[i];
                miny = v2[j];
              } else if (((v1[i] * v2[j]) == (minx * miny)))
              {
                if ((v1[i] <= minx))
                {
                  minx = v1[i];
                  miny = v2[j];
                }
              }
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    printf("%d\n", ans);
    printf("%d %d\n", minx, miny);
  }
  return 0;
}
