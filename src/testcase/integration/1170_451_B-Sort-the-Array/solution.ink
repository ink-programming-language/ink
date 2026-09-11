// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var l: dynamic = cpp_uninitialized();

var r: dynamic = cpp_uninitialized();

var i: dynamic = cpp_uninitialized();

var sl: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(1000000);

var b: dynamic = cpp_array(1000000);

var que: dynamic = cpp_uninitialized();

var tk: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(n);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(a[i]);
      b[i] = a[i];
      i += 1;
    }
  }
  sort((b + 1), ((b + 1) + n));
  var dd: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((dd == 1))
      {
        que.push(a[i]);
        tk.push(b[i]);
        var kt: dynamic = 0;
        while ((((!que.empty()) && (!tk.empty())) && (que.front() == tk.top())))
        {
          que.pop();
          tk.pop();
          kt = 1;
          r = i;
        }
        if ((kt == 1))
        {
          if (((!que.empty()) || (!tk.empty())))
          {
            sl += 1;
          }
          dd = 0;
        }
        sl += kt;
      } else if (((a[i] != b[i]) && (dd == 0)))
      {
        que.push(a[i]);
        tk.push(b[i]);
        dd = 1;
        l = i;
      }
      i += 1;
    }
  }
  if ((((!que.empty()) || (!tk.empty())) || (sl > 1)))
  {
    write("no");
  } else
  {
    if ((sl == 0))
    {
      write("yes", "\n");
      write(1, " ", 1);
    } else
    {
      write("yes", "\n");
      write(l, " ", r);
    }
  }
  return 0;
}
