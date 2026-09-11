// Translated from solution.cpp.

func main() -> dynamic
{
  var z: dynamic = cpp_uninitialized();
  var o: dynamic = cpp_uninitialized();
  scanf("%d %d", (&z), (&o));
  var max1: dynamic = (4 + (2 * ((z - 1))));
  if ((((o < (z - 1))) || ((o > max1))))
  {
    puts("-1");
    return 0;
  }
  var mid: dynamic = (z - 1);
  var rem: dynamic = (o - mid);
  if ((rem > 4))
  {
    mid += (rem - 4);
    rem = 4;
  }
  var exr: dynamic = ((mid - z) + 1);
  {
    typeof((rem / 2)) = 0;
    while ((i < (rem / 2)))
    {
      printf("1");
      i += 1;
    }
  }
  {
    typeof(z) = 0;
    while ((i < z))
    {
      if (i)
      {
        printf("1");
        if ((exr > 0))
        {
          exr -= 1;
          printf("1");
        }
      }
      printf("0");
      i += 1;
    }
  }
  {
    typeof((rem - ((rem / 2)))) = 0;
    while ((i < (rem - ((rem / 2)))))
    {
      printf("1");
      i += 1;
    }
  }
  return 0;
}
