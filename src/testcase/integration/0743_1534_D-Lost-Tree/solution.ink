// Translated from solution.cpp.

var ll = dynamic;

var mex = cpp_expression("#inclu");

var pi = cpp_expression("#include<bits");

func minmax(a: dynamic, b: dynamic)
{
  cpp_macro("{min(a,b),max(a,b)}");
}

func main()
{
  var n: dynamic;
  read(n);
  write("? 1\n");
  cout.flush();
  var a = cpp_array(n);
  var h: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      h[a[i]].push_back(i);
      i += 1;
    }
  }
  var ed: dynamic;
  var oc = 0;
  var ec = 0;
  {
    var i = 0;
    while ((i < n))
    {
      if ((i & 1))
      {
        oc += h[i].size();
      } else
      {
        ec += h[i].size();
      }
      i += 1;
    }
  }
  var in_cpp = 2;
  if ((ec > oc))
  {
    in_cpp -= 1;
  }
  for (var x in h[1])
  {
    ed.insert([0, x]);
  }
  {
    var i = in_cpp;
    while ((i < n))
    {
      if ((ed.size() == (n - 1)))
      {
        break;
      }
      for (var x in h[i])
      {
        write("? ", (x + 1), cpp_char("\n"));
        cout.flush();
        {
          var i = 0;
          while ((i < n))
          {
            read(a[i]);
            if ((a[i] == 1))
            {
              ed.insert(minmax(x, i));
            }
            i += 1;
          }
        }
      }
      i += 2;
    }
  }
  write("!\n");
  var it = ed.begin();
  while ((it != ed.end()))
  {
    write((((*it)).first + 1), cpp_char(" "), (((*it)).second + 1), cpp_char("\n"));
    it += 1;
  }
  return 0;
}
