// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var s: dynamic = cpp_array(501);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(s[i]);
      i += 1;
    }
  }
  var ctt: dynamic = 0;
  var nuee: dynamic = cpp_array(501);
  fill(nuee, (nuee + n), 1);
  var dame: dynamic = 1;
  {
    var j: dynamic = 0;
    while ((j < n))
    {
      var ok: dynamic = 1;
      {
        var i: dynamic = 0;
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
  var ans: dynamic = (2 * n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var cty: dynamic = 0;
      {
        var j: dynamic = 0;
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
