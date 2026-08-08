// Translated from solution.cpp.

func main()
{
  var K: dynamic;
  var A: dynamic;
  var B: dynamic;
  read(K, A, B);
  if ((K <= A))
  {
    write(1, "\n");
  } else if (((A - B) <= 0))
  {
    write(-1, "\n");
  } else
  {
    var t = (K - A);
    write(((((((t + ((A - B))) - 1)) / ((A - B))) * 2) + 1), "\n");
  }
}
