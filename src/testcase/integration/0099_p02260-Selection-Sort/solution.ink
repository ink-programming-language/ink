// Translated from solution.cpp.

func main(argument_0: dynamic)
{
  var n: dynamic;
  var an = cpp_array(100);
  var cnt = 0;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(an[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      var mini = i;
      var tmp = an[i];
      {
        var j = i;
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
    var i = 0;
    while ((i < n))
    {
      write(an[i], (if ((i == (n - 1))) "\n" else " "));
      i += 1;
    }
  }
  write(cnt, "\n");
  return 0;
}
