// Translated from solution.cpp.

var n: dynamic;

func main()
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  read(n);
  if ((n <= 50000))
  {
    write("3\n");
    exit(0);
  }
  if ((n >= 54000))
  {
    write("1\n");
    exit(0);
  }
  write("2\n");
  return 0;
}
