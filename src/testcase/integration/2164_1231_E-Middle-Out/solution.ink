// Translated from solution.cpp.

func check(a: dynamic, b: dynamic) -> dynamic
{
  sort(a.begin(), a.end());
  sort(b.begin(), b.end());
  return (a == b);
}

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    read(n);
    var s: dynamic = cpp_uninitialized();
    var p: dynamic = cpp_uninitialized();
    read(s, p);
    if ((!check(s, p)))
    {
      write(-1, "\n");
      continue;
    }
    var ans: dynamic = s.size();
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
