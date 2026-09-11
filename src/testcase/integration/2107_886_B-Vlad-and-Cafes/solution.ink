// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(200005);

var myset: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(n);
  {
    var i: dynamic = (0);
    var b: dynamic = ((n - 1));
    while ((i <= b))
    {
      read(a[i]);
      i += 1;
    }
  }
  var res: dynamic = -1;
  {
    var i: dynamic = ((n - 1));
    var b: dynamic = (0);
    while ((i >= b))
    {
      var pre: dynamic = myset.size();
      myset.insert(a[i]);
      if ((myset.size() > pre))
      {
        res = a[i];
      }
      i -= 1;
    }
  }
  write(res);
  return 0;
}

func checkDefine() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_array(200005);
  var m: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = (0);
    var b: dynamic = ((n - 1));
    while ((i <= b))
    {
      read(a[i]);
      m[a[i]] += 1;
      i += 1;
    }
  }
  var s: dynamic = cpp_uninitialized();
  read(s);
  {
    write("s", " = ");
    write((s), "\n");
  }
  {
    write("a", " = ");
    {
      var cpp_name: dynamic = 0;
      var a: dynamic = (n);
      while ((cpp_name < a))
      {
        write(a[cpp_name], cpp_char(" "));
        cpp_name += 1;
      }
    }
    write("\n");
  }
  {
    write("\"------------\"", " = ");
    write(("------------"), "\n");
  }
  {
    typeof(m.begin()) = m.begin();
    while ((it != m.end()))
    {
      write(it->first, " ", it->second, "\n");
      it += 1;
    }
  }
}
