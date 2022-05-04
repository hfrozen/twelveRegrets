import 'package:flutter/material.dart';
import 'package:flutter_colorpicker/flutter_colorpicker.dart';
import 'package:papy/refer/const.dart';
import 'package:papy/refer/translates.dart';

class DialogWidget extends StatelessWidget {
  const DialogWidget({
    required this.text,
    required this.confirm,
    required this.decline,
  });

  final String text;
  final void Function() confirm;
  final void Function() decline;

  @override
  Widget build(BuildContext context) {
    return AlertDialog(
      content: Text(text),
      actions: [
        TextButton(
          child: Text(Localization.of(context)!.tr('yes')!),
          onPressed: confirm,
        ),
        TextButton(
          child: Text(Localization.of(context)!.tr('No')!),
          onPressed: decline,
        ),
      ],
    );
  }
}

class ColorPickerDialog extends StatelessWidget {
  const ColorPickerDialog({
    required this.foreOrBack,
    required this.currentColor,
    required this.onColorChanged,
  });

  final bool foreOrBack;
  final Color currentColor;
  final ValueChanged<Color> onColorChanged;

  @override
  Widget build(BuildContext context) {
    return AlertDialog(
      title: Text(foreOrBack ? 'foreground color' : 'background color'),
      content: SingleChildScrollView(
        child: BlockPicker(
          onColorChanged: onColorChanged,
          pickerColor: currentColor,
          availableColors: foreOrBack
              ? Environs().foregroundColors
              : Environs().backgroundColors,
          layoutBuilder: layoutBuilder,
          itemBuilder: itemBuilder,
        ),
      ),
    );
  }

  Widget layoutBuilder(BuildContext context,
      List<Color> colors, PickerItem child) {
    return SizedBox(
      width: 300,
      height: 360,
      child: GridView.count(
        crossAxisCount: 4,
        crossAxisSpacing: 2,
        mainAxisSpacing: 2,
        children: [for (Color color in colors) child(color)],
      ),
    );
  }

  Widget itemBuilder(Color color,
      bool isCurrentColor, void Function() changeColor) {
    return Container(
      margin: const EdgeInsets.all(4),
      decoration: BoxDecoration(
        borderRadius: BorderRadius.circular(2),
        color: color,
        boxShadow: [BoxShadow(
          color: color.withOpacity(0.8),
          offset: const Offset(1, 2),
          blurRadius: 4,
        )],
      ),
      child: Material(
        color: Colors.transparent,
        child: InkWell(
          onTap: changeColor,
          borderRadius: BorderRadius.circular(2),
          child: AnimatedOpacity(
            duration: const Duration(milliseconds: 250),
            opacity: isCurrentColor ? 1 : 0,
            child: Icon(
              Icons.done,
              size: 24,
              color: useWhiteForeground(color) ? Colors.white : Colors.black,
            ),
          ),
        ),
      ),
    );
  }
}

class ColorPicker extends StatelessWidget {
  const ColorPicker({
    required this.foreOrBack,
    required this.currentColor,
    required this.backgroundColor,
    required this.height,
    required this.onColorChanged,
  });

  final bool foreOrBack;
  final Color currentColor;
  final Color backgroundColor;
  final double height;
  final ValueChanged<Color> onColorChanged;

  @override
  Widget build(BuildContext context) {
    return BlockPicker(
      onColorChanged: onColorChanged,
      pickerColor: currentColor,
      availableColors: foreOrBack
          ? Environs().foregroundColors
          : Environs().backgroundColors,
      layoutBuilder: layoutBuilder,
      itemBuilder: itemBuilder,
    );
  }
  Widget layoutBuilder(BuildContext context,
      List<Color> colors, PickerItem child) {
    return Container(
      width: MediaQuery.of(context).size.width,
      color: backgroundColor,
      height: height,
      child: GridView.count(
        crossAxisCount: Environs().backgroundColors.length,
        crossAxisSpacing: 2,
        mainAxisSpacing: 2,
        children: [for (Color color in colors) child(color)],
      ),
    );
  }

  Widget itemBuilder(Color color,
      bool isCurrentColor, void Function() changeColor) {
    return Container(
      margin: const EdgeInsets.all(4),
      decoration: BoxDecoration(
        borderRadius: BorderRadius.circular(4),
        color: color,
        border: Border.all(
          color: Colors.black,
          style: BorderStyle.solid,
          width: 2,
        ),
        boxShadow: [BoxShadow(
          color: color.withOpacity(1.0),
          //offset: const Offset(1, 2),
          //blurRadius: 4,
        )],
      ),
      child: Material(
        color: Colors.transparent,
        child: InkWell(
          onTap: changeColor,
          borderRadius: BorderRadius.circular(2),
          child: AnimatedOpacity(
            duration: const Duration(milliseconds: 250),
            opacity: isCurrentColor ? 1 : 0,
            child: Icon(
              Icons.done,
              size: 24,
              color: useWhiteForeground(color) ? Colors.white : Colors.black,
            ),
          ),
        ),
      ),
    );
  }
}