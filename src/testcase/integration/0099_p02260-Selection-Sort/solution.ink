// Translated from solution.cpp.

func main(argument_0: dynamic) -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var an: dynamic = cpp_array(100);
  var cnt: dynamic = 0;
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(an[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var mini: dynamic = i;
      var tmp: dynamic = an[i];
      {
        var j: dynamic = i;
        while ((j < n))
        {
          if ((an[j] < an[mini]))
          {
            mini = j;
          }
          j += 1;
        }
      }
      if ((i != mini))
      {
        an[i] = an[mini];
        an[mini] = tmp;
        cnt += 1;
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      write(an[i], ( ((i == (n - 1))) ? "\n" : " "));
      i += 1;
    }
  }
  write(cnt, "\n");
  return 0;
}
