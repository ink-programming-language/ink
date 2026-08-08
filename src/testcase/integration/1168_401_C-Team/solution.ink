// Translated from solution.cpp.

func main()
{
  var z: dynamic;
  var o: dynamic;
  scanf("%d %d", (&z), (&o));
  var max1 = (4 + (2 * ((z - 1))));
  if ((((o < (z - 1))) || ((o > max1))))
  {
    puts("-1");
    return 0;
  }
  var mid = (z - 1);
  var rem = (o - mid);
  if ((rem > 4))
  {
    mid += (rem - 4);
    rem = 4;
  }
  var exr = ((mid - z) + 1);
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
