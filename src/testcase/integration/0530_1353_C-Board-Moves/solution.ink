// Translated from solution.cpp.

func main()
{
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    read(n);
    var sum = 0;
    var j = (((n - 1)) / 2);
    {
      var i = 0;
      while ((i < j))
      {
        sum += (pow((i + 1), 2) * 8);
        i += 1;
      }
    }
    write(sum, "\n");
  }
}
