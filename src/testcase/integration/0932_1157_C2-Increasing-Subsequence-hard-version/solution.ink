// Translated from solution.cpp.

func main()
{
  var i: dynamic;
  var c = 0;
  var n: dynamic;
  var T: dynamic;
  var cs = 0;
  var k: dynamic;
  var x: dynamic;
  var j: dynamic;
  T = 1;
  var s = "";
  while (cpp_update(T, "--"))
  {
    read(n);
    var a = cpp_array(n);
    {
      i = 0;
      while ((i < n))
      {
        read(a[i]);
        i += 1;
      }
    }
    i = (n - 1);
    j = 0;
    c = 0;
    while ((i >= j))
    {
      if (((a[i] == a[j]) && (i != j)))
      {
        k = j;
        cs = c;
        var p = "";
        var q = "";
        while (((j < i) && (a[j] > c)))
        {
          p += cpp_char("L");
          c = a[j];
          j += 1;
        }
        c = cs;
        while (((i > k) && (a[i] > c)))
        {
          q += cpp_char("R");
          c = a[i];
          i -= 1;
        }
        if ((p.size() > q.size()))
        {
          s += p;
        } else
        {
          s += q;
        }
        break;
      } else if (((a[i] > a[j]) && (a[j] > c)))
      {
        s += cpp_char("L");
        c = a[j];
        j += 1;
      } else if ((a[i] > c))
      {
        s += cpp_char("R");
        c = a[i];
        i -= 1;
      } else if ((a[j] > c))
      {
        s += cpp_char("L");
        c = a[j];
        j += 1;
      } else
      {
        break;
      }
    }
    write(s.size(), "\n", s);
  }
  return 0;
}
