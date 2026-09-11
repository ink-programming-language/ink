// Translated from solution.cpp.

var lat: dynamic = cpp_array(5, 5);

var bx: dynamic = [[0, 1, 2, 3, 4], [0, 1, 2, 3, 4], [0, 1, 2, 3, 4], [0, 1, 2, 3, 4], [0, 1, 2, 3, 4], [0, 0, 0, 0, 0], [1, 1, 1, 1, 1], [2, 2, 2, 2, 2], [3, 3, 3, 3, 3], [4, 4, 4, 4, 4]];

var by: dynamic = [[0, 0, 0, 0, 0], [1, 1, 1, 1, 1], [2, 2, 2, 2, 2], [3, 3, 3, 3, 3], [4, 4, 4, 4, 4], [0, 1, 2, 3, 4], [0, 1, 2, 3, 4], [0, 1, 2, 3, 4], [0, 1, 2, 3, 4], [0, 1, 2, 3, 4]];

func rev(s: dynamic) -> dynamic
{
  reverse(s.begin(), s.end());
  return s;
}

func dfs(v: dynamic, k: dynamic) -> dynamic
{
  if ((k == 10))
  {
    return 1;
  }
  var S: dynamic = cpp_construct(1, v[k]);
  if ((v[k] != rev(v[k])))
  {
    S.push_back(rev(v[k]));
  }
  var ret: dynamic = 0;
  for (var bloc: dynamic in S)
  {
    {
      var i: dynamic = 0;
      while ((i < 10))
      {
        var ok: dynamic = true;
        var t: dynamic = cpp_uninitialized();
        {
          var j: dynamic = 0;
          while ((j < 5))
          {
            if (((((lat[by[i][j]][bx[i][j]] & 1) && ((bloc[j] - cpp_char("0")) == 0))) || (((lat[by[i][j]][bx[i][j]] & 2) && ((bloc[j] - cpp_char("0")) == 1)))))
            {
              ok = false;
              break;
            }
            t.push_back(lat[by[i][j]][bx[i][j]]);
            j += 1;
          }
        }
        if (ok)
        {
          {
            var j: dynamic = 0;
            while ((j < 5))
            {
              lat[by[i][j]][bx[i][j]] |= (((bloc[j] - cpp_char("0")) + 1));
              j += 1;
            }
          }
          ret += dfs(v, (k + 1));
          {
            var j: dynamic = 0;
            while ((j < 5))
            {
              lat[by[i][j]][bx[i][j]] = t[j];
              j += 1;
            }
          }
        }
        i += 1;
      }
    }
  }
  return ret;
}

func main() -> dynamic
{
  var v: dynamic = cpp_construct(10);
  while (cpp_comma((cin >> v[0]), (v[0] != "END")))
  {
    {
      var i: dynamic = 1;
      while ((i < 10))
      {
        read(v[i]);
        i += 1;
      }
    }
    memset(lat, 0, cpp_sizeof(lat));
    var ret: dynamic = dfs(v, 0);
    write(( ((ret == 0)) ? 0 : (ret / 8)), "\n");
  }
  return 0;
}
