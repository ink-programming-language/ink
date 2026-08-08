// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var mini = 33;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      mini = min(mini, a[i]);
      i += 1;
    }
  }
  write((2 + ((a[2] ^ mini))), "\n");
  return 0;
}
