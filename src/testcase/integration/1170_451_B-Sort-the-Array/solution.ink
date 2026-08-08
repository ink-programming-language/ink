// Translated from solution.cpp.

var n: dynamic;

var l: dynamic;

var r: dynamic;

var i: dynamic;

var sl: dynamic;

var a = cpp_array(1000000);

var b = cpp_array(1000000);

var que: dynamic;

var tk: dynamic;

func main()
{
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      read(a[i]);
      b[i] = a[i];
      i += 1;
    }
  }
  sort((b + 1), ((b + 1) + n));
  var dd = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      if ((dd == 1))
      {
        que.push(a[i]);
        tk.push(b[i]);
        var kt = 0;
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
