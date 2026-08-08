// Translated from solution.cpp.

var n: dynamic;

var a = cpp_array(200005);

var myset: dynamic;

func main()
{
  read(n);
  {
    var i = (0);
    var b = ((n - 1));
    while ((i <= b))
    {
      read(a[i]);
      i += 1;
    }
  }
  var res = -1;
  {
    var i = ((n - 1));
    var b = (0);
    while ((i >= b))
    {
      var pre = myset.size();
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

func checkDefine()
{
  var n: dynamic;
  var a = cpp_array(200005);
  var m: dynamic;
  read(n);
  {
    var i = (0);
    var b = ((n - 1));
    while ((i <= b))
    {
      read(a[i]);
      m[a[i]] += 1;
      i += 1;
    }
  }
  var s: dynamic;
  read(s);
  {
    write("s", " = ");
    write((s), "\n");
  }
  {
    write("a", " = ");
    {
      var cpp_name = 0;
      var a = (n);
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
