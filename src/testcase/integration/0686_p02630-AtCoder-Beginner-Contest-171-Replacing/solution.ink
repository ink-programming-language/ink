// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  var s: dynamic = 0;
  read(n);
  var a: dynamic = cpp_array(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  read(q);
  var p: dynamic = [0];
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      s += a[i];
      p[a[i]] += 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < q))
    {
      var b: dynamic = cpp_uninitialized();
      var c: dynamic = cpp_uninitialized();
      read(b, c);
      s += (((c - b)) * p[b]);
      p[c] += p[b];
      p[b] = 0;
      write(s, "\n");
      i += 1;
    }
  }
  return 0;
}
