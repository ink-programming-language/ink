// Translated from solution.cpp.

var lat = cpp_array(5, 5);

var bx = [[0, 1, 2, 3, 4], [0, 1, 2, 3, 4], [0, 1, 2, 3, 4], [0, 1, 2, 3, 4], [0, 1, 2, 3, 4], [0, 0, 0, 0, 0], [1, 1, 1, 1, 1], [2, 2, 2, 2, 2], [3, 3, 3, 3, 3], [4, 4, 4, 4, 4]];

var by = [[0, 0, 0, 0, 0], [1, 1, 1, 1, 1], [2, 2, 2, 2, 2], [3, 3, 3, 3, 3], [4, 4, 4, 4, 4], [0, 1, 2, 3, 4], [0, 1, 2, 3, 4], [0, 1, 2, 3, 4], [0, 1, 2, 3, 4], [0, 1, 2, 3, 4]];

func rev(s: dynamic)
{
  reverse(s.begin(), s.end());
  return s;
}

func dfs(v: dynamic, k: dynamic)
{
  if ((k == 10))
  {
    return 1;
  }
  var S = cpp_construct(1, v[k]);
  if ((v[k] != rev(v[k])))
  {
    S.push_back(rev(v[k]));
  }
  var ret = 0;
  for (var bloc in S)
  {
    {
      var i = 0;
      while ((i < 10))
      {
        var ok = true;
        var t: dynamic;
        {
          var j = 0;
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
            var j = 0;
            while ((j < 5))
            {
              lat[by[i][j]][bx[i][j]] |= (((bloc[j] - cpp_char("0")) + 1));
              j += 1;
            }
          }
          ret += dfs(v, (k + 1));
          {
            var j = 0;
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

func main()
{
  var v = cpp_construct(10);
  while (cpp_comma((cin >> v[0]), (v[0] != "END")))
  {
    {
      var i = 1;
      while ((i < 10))
      {
        read(v[i]);
        i += 1;
      }
    }
    memset(lat, 0, cpp_sizeof(lat));
    var ret = dfs(v, 0);
    write((if ((ret == 0)) 0 else (ret / 8)), "\n");
  }
  return 0;
}
