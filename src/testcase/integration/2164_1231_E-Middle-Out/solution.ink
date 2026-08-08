// Translated from solution.cpp.

func check(a: dynamic, b: dynamic)
{
  sort(a.begin(), a.end());
  sort(b.begin(), b.end());
  return (a == b);
}

func main()
{
  var t: dynamic;
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  var l: dynamic;
  var n: dynamic;
  var m: dynamic;
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  var x: dynamic;
  var y: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    read(n);
    var s: dynamic;
    var p: dynamic;
    read(s, p);
    if ((!check(s, p)))
    {
      write(-1, "\n");
      continue;
    }
    var ans = s.size();
    n = s.size();
    {
      i = 0;
      while ((i < n))
      {
        j = 0;
        l = i;
        while ((j < n))
        {
          if ((s[j] == p[l]))
          {
            j += 1;
            l += 1;
          } else
          {
            j += 1;
          }
        }
        ans = min(ans, ((i + n) - l));
        i += 1;
      }
    }
    write(ans, "\n");
  }
  return 0;
}
