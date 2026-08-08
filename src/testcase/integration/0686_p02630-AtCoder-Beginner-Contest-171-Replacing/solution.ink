// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var q: dynamic;
  var s = 0;
  read(n);
  var a = cpp_array(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  read(q);
  var p = [0];
  {
    var i = 0;
    while ((i < n))
    {
      s += a[i];
      p[a[i]] += 1;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < q))
    {
      var b: dynamic;
      var c: dynamic;
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
