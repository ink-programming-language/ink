// Translated from solution.cpp.

func main(argument_0: dynamic) -> dynamic
{
  var c: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_array(50);
  read(c);
  var n: dynamic = cpp_array(50);
  {
    var i: dynamic = 0;
    while ((i < c))
    {
      read(n[i]);
      read(m[i]);
      i += 1;
    }
  }
  var fm: dynamic = cpp_uninitialized();
  read(fm);
  var j: dynamic = cpp_uninitialized();
  var sum: dynamic = 0;
  {
    j = 0;
    while ((j < c))
    {
      if ((fm == n[j]))
      {
        j += 1;
        break;
      }
      j += 1;
    }
  }
  {
    j = j;
    while ((j < c))
    {
      sum = (sum + m[j]);
      j += 1;
    }
  }
  write(sum, "\n");
  return 0;
}
