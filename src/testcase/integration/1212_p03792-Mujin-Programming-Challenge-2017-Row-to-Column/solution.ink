// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  read(n);
  var s = cpp_array(501);
  {
    var i = 0;
    while ((i < n))
    {
      read(s[i]);
      i += 1;
    }
  }
  var ctt = 0;
  var nuee = cpp_array(501);
  fill(nuee, (nuee + n), 1);
  var dame = 1;
  {
    var j = 0;
    while ((j < n))
    {
      var ok = 1;
      {
        var i = 0;
        while ((i < n))
        {
          if ((s[i][j] == cpp_char(".")))
          {
            ok = 0;
          } else
          {
            nuee[j] = 0;
          }
          i += 1;
        }
      }
      if (ok)
      {
        ctt += 1;
      }
      if ((!nuee[j]))
      {
        dame = 0;
      }
      j += 1;
    }
  }
  if (dame)
  {
    write(-1, "\n");
    return 0;
  }
  var ans = (2 * n);
  {
    var i = 0;
    while ((i < n))
    {
      var cty = 0;
      {
        var j = 0;
        while ((j < n))
        {
          if ((s[i][j] == cpp_char("#")))
          {
            cty += 1;
          }
          j += 1;
        }
      }
      if ((cty == n))
      {
        write((n - ctt), "\n");
        return 0;
      }
      if (nuee[i])
      {
        ans = min(ans, ((((n - ctt) + n) - cty) + 1));
      } else
      {
        ans = min(ans, (((n - ctt) + n) - cty));
      }
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
